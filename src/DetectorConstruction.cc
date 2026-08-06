#include "DetectorConstruction.hh"
#include "SiPMSD.hh"

#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4SubtractionSolid.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4SDManager.hh"
#include "G4UserLimits.hh"
#include "G4MaterialPropertiesTable.hh"
#include "G4OpticalSurface.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4LogicalBorderSurface.hh"
#include <cmath>

DetectorConstruction::DetectorConstruction() {
    fMessenger = new DetectorMessenger(this);
}
DetectorConstruction::~DetectorConstruction() {
    delete fMessenger;
}

// ─────────────────────────────────────────────────────────────────
// NOTE: this now only touches the DICLAD *skin* surface (reflect /
// TPB coating reflectivity at the DICLAD boundary) plus the bulk
// DICLAD absorption length in "absorb" mode. It assumes fDicladLogical
// has already been set to the real placed volume (done in Construct()).
// ─────────────────────────────────────────────────────────────────
void DetectorConstruction::SetDicladMode(const G4String& mode)
{
    fDicladMode = mode;

    static G4LogicalSkinSurface* dicladSkin = nullptr;
    delete dicladSkin;
    dicladSkin = nullptr;

    if (mode == "reflect") {

        auto* reflectSurface = new G4OpticalSurface("DICLADReflectiveSurface");
        reflectSurface->SetType(dielectric_dielectric);
        reflectSurface->SetFinish(groundfrontpainted);
        reflectSurface->SetModel(unified);
        reflectSurface->SetSigmaAlpha(0.1);

        std::vector<G4double> energies = {6.0*eV, 7.08*eV, 10.0*eV};
        std::vector<G4double> rindex_diclad  = {1.41, 1.41, 1.41};
        std::vector<G4double> abslen_diclad  = {0.1*mm, 0.1*mm, 0.1*mm};
        std::vector<G4double> reflectivity   = {0.68, 0.68, 0.68};
        std::vector<G4double> zero_vector    = {0.0,  0.0,  0.0 };
        std::vector<G4double> diffuse_vector = {1.0,  1.0,  1.0 };

        auto* reflectMPT = new G4MaterialPropertiesTable();
        reflectMPT->AddProperty("REFLECTIVITY",          energies, reflectivity,   true);
        reflectMPT->AddProperty("SPECULARSPIKECONSTANT", energies, zero_vector,    true);
        reflectMPT->AddProperty("SPECULARLOBECONSTANT",  energies, zero_vector,    true);
        reflectMPT->AddProperty("BACKSCATTERCONSTANT",   energies, zero_vector,    true);
        reflectMPT->AddProperty("DIFFUSELOBECONSTANT",   energies, diffuse_vector, true);
        reflectMPT->AddProperty("RINDEX",    energies, rindex_diclad);
        reflectMPT->AddProperty("ABSLENGTH", energies, abslen_diclad);
        reflectSurface->SetMaterialPropertiesTable(reflectMPT);

        dicladSkin = new G4LogicalSkinSurface("dicladOpticalSkin", fDicladLogical, reflectSurface);

        G4cout << "[Detector] DICLAD mode: REFLECTIVE (Lambertian teflon, R=0.68)" << G4endl;

    } else if (mode == "TPB" || mode == "tpb") {

        // Reflectivity/diffuse behaviour of light hitting the DICLAD
        // substrate through the TPB layer. This governs the DICLAD
        // *surface*, not the TPB bulk (WLS lives on fTpbMat instead,
        // see DefineMaterials()).
        
        auto* tpbSurface = new G4OpticalSurface("TPBDicladReflectiveSurface");
        tpbSurface->SetType(dielectric_dielectric);
        tpbSurface->SetFinish(groundfrontpainted);
        tpbSurface->SetModel(unified);
        tpbSurface->SetSigmaAlpha(0.1);

        std::vector<G4double> energies = {6.0*eV, 7.08*eV, 10.0*eV};
        std::vector<G4double> reflectivity   = {0.99, 0.99, 0.99};
        std::vector<G4double> zero_vector    = {0.0,  0.0,  0.0 };
        std::vector<G4double> diffuse_vector = {1.0,  1.0,  1.0 };

        auto* tpbSurfaceMPT = new G4MaterialPropertiesTable();
        tpbSurfaceMPT->AddProperty("REFLECTIVITY",          energies, zero_vector,   true);
        tpbSurfaceMPT->AddProperty("SPECULARSPIKECONSTANT", energies, zero_vector,    true);
        tpbSurfaceMPT->AddProperty("SPECULARLOBECONSTANT",  energies, zero_vector,    true);
        tpbSurfaceMPT->AddProperty("BACKSCATTERCONSTANT",   energies, zero_vector,    true);
        tpbSurfaceMPT->AddProperty("DIFFUSELOBECONSTANT",   energies, diffuse_vector, true);
        tpbSurface->SetMaterialPropertiesTable(tpbSurfaceMPT);

        dicladSkin = new G4LogicalSkinSurface("dicladOpticalSkin", fDicladLogical, tpbSurface);

        G4cout << "[Detector] DICLAD mode: TPB coating (WLS in TPB bulk + Lambertian R=0.99 at TPB-DICLAD interface)" << G4endl;

    } else {
        SetOpaqueOpticalProperties(fDicladMat, 1.0*nm);
        G4cout << "[Detector] DICLAD mode: ABSORBING" << G4endl;
    }
}

void DetectorConstruction::DefineMaterials()
{
    G4NistManager* nist = G4NistManager::Instance();

    // Xenon gas at 4 bar, 293 K
    G4double pressure    = 4.0 * bar;
    G4double temperature = 293.15 * kelvin;
    G4double density     = 4.0 * 5.858e-3 * g/cm3; // ~4x NTP density

    fXenonGas = new G4Material("XenonGas4bar", density, 1,
                                kStateGas, temperature, pressure);
    G4Element* Xe = nist->FindOrBuildElement("Xe");
    fXenonGas->AddElement(Xe, 1);

    // DICLAD substrate for the GALA body
    auto* C  = nist->FindOrBuildElement("C");
    auto* H  = nist->FindOrBuildElement("H");
    auto* O  = nist->FindOrBuildElement("O");
    auto* Si = nist->FindOrBuildElement("Si");

    fDicladMat = new G4Material("DICLAD", 1.85 * g/cm3, 4, kStateSolid);
    fDicladMat->AddElement(C,  18.0 * perCent);
    fDicladMat->AddElement(H,   4.0 * perCent);
    fDicladMat->AddElement(O,  38.0 * perCent);
    fDicladMat->AddElement(Si, 40.0 * perCent);

    // Silicon for SiPMs
    fSiPMMat = nist->FindOrBuildMaterial("G4_Si");

    // Vacuum / world
    fVacuum = nist->FindOrBuildMaterial("G4_Galactic");

    auto* tpb = new G4Material("TPB", 1.2 * g/cm3, 2, kStateSolid);
    tpb->AddElement(C, 28);
    tpb->AddElement(H, 22);
    fTpbMat = tpb;

    // Optical properties: hole gas propagates photons, materials absorb them.
    SetOpaqueOpticalProperties(fXenonGas, 1000.0 * m);
    SetOpaqueOpticalProperties(fVacuum,    1000.0 * m);
    SetOpaqueOpticalProperties(fSiPMMat,   1.0 * nm);

    // ── TPB BULK optical properties ─────────────────────────────
    // These MUST live on the material itself (fTpbMat), not on a
    // border/skin surface: G4OpWLS reads WLSABSLENGTH/WLSCOMPONENT
    // from the logical volume's *material* properties table, and
    // G4OpBoundaryProcess reads RINDEX from the two volumes' own
    // materials to compute Fresnel refraction/reflection at a
    // dielectric_dielectric boundary. A surface MPT is the wrong
    // place for any of this.
    std::vector<G4double> rindexEnergies = {
        2.0*eV, 2.4*eV, 2.7*eV, 2.9*eV, 3.1*eV, 3.4*eV,
        6.0*eV, 7.08*eV, 10.0*eV
    };
    std::vector<G4double> rindex_tpb = {1.67, 1.67, 1.67, 1.67, 1.67, 1.67, 1.67, 1.67, 1.67};
    std::vector<G4double> abslen_tpb = {100.0*m, 100.0*m, 100.0*m, 100.0*m, 100.0*m, 100.0*m, 5.0*nm, 5.0*nm, 5.0*nm};
    std::vector<G4double> wlsAbslenEnergy   = {6.0*eV, 7.08*eV, 10.0*eV};
    std::vector<G4double> wlsAbslen_tpb     = {5.0*nm, 5.0*nm, 5.0*nm};
    std::vector<G4double> wlsEmissionEnergy = {2.4*eV, 2.7*eV, 2.9*eV, 3.05*eV, 3.2*eV, 3.4*eV};
    std::vector<G4double> wlsEmission       = {0.0, 0.15, 0.65, 1.0, 0.55, 0.1};

    auto* tpbMPT = new G4MaterialPropertiesTable();
    tpbMPT->AddProperty("RINDEX",       rindexEnergies,   rindex_tpb);
    tpbMPT->AddProperty("ABSLENGTH",    rindexEnergies,   abslen_tpb);
    tpbMPT->AddProperty("WLSABSLENGTH", wlsAbslenEnergy,  wlsAbslen_tpb);
    tpbMPT->AddProperty("WLSCOMPONENT", wlsEmissionEnergy, wlsEmission);
    tpbMPT->AddConstProperty("WLSMEANNUMBERPHOTONS", 1.0);
    tpbMPT->AddConstProperty("WLSTIMECONSTANT", 1.68 * ns);
    fTpbMat->SetMaterialPropertiesTable(tpbMPT);

    // No physical volumes exist yet at this point (Construct() calls
    // DefineMaterials() first) — the TPB/TPC border surface is built
    // in Construct() itself, once fTpbPhys and fTpcPhys are real.
}

G4VPhysicalVolume* DetectorConstruction::Construct()
{
    DefineMaterials();

    // ─── World ───────────────────────────────────────────────────
    G4double worldSize = 600.0*m;
    auto* worldSolid = new G4Box("World", worldSize/2, worldSize/2, worldSize/2);
    auto* worldLogic = new G4LogicalVolume(worldSolid, fVacuum, "World");
    auto* worldPhys  = new G4PVPlacement(nullptr, {}, worldLogic, "World", nullptr, false, 0);
    worldLogic->SetVisAttributes(G4VisAttributes::GetInvisible());

    // ─── TPC Barrel (cylindrical, Xenon gas) ─────────────────────
    G4double rInner = 0;
    G4double rOuter = kHoleDiameter * mm;
    G4double halfH  = kGALAThickness / 2.0 * mm;
    const G4double tpbThickness = kTpbThickness * um;
    const G4double tpbInnerRadius = rOuter / 2.0 - tpbThickness;

    const G4double dicladOuterXY = 20.0 * mm;
    auto* dicladBox = new G4Box("DICLADBox", dicladOuterXY / 2.0, dicladOuterXY / 2.0, halfH);
    auto* dicladHole = new G4Tubs("DICLADHole", rInner, rOuter/2, halfH + 1.0 * um, 0, 360 * deg);
    auto* dicladSolid = new G4SubtractionSolid("DICLADSolid", dicladBox, dicladHole, nullptr, G4ThreeVector());

    auto* dicladLogic = new G4LogicalVolume(dicladSolid, fDicladMat, "DICLAD");
    fDicladLogical = dicladLogic;   // <-- FIX: keep the member in sync with the real volume
    fDicladPhys = new G4PVPlacement(nullptr, G4ThreeVector(0,0,0),
                                    dicladLogic, "DICLAD", worldLogic, false, 0);

    G4VisAttributes* dicladVis = new G4VisAttributes(G4Colour(0.55, 0.35, 0.2, 0.65));
    dicladVis->SetForceSolid(true);
    dicladLogic->SetVisAttributes(dicladVis);

    const G4bool hasCoating = (fDicladMode == "TPB" || fDicladMode == "tpb");
    const G4double tpcRadius = hasCoating
        ? tpbInnerRadius
        : rOuter / 2.0;
    auto* tpcSolid  = new G4Tubs("TPC", rInner, tpcRadius, halfH, 0, 360*deg);
    auto* tpcLogic  = new G4LogicalVolume(tpcSolid, fXenonGas, "TPC");
    fTpcPhys = new G4PVPlacement(nullptr, G4ThreeVector(0,0,0),
                                    tpcLogic, "TPC", worldLogic, false, 0);

    G4VisAttributes* tpcVis = new G4VisAttributes(G4Colour(0.5,0.8,1.0,0.15));
    tpcVis->SetForceSolid(true);
    tpcLogic->SetVisAttributes(tpcVis);

    const G4double sipmHalfXY = (kSiPMSize / 2.0) * mm;
    const G4double sipmHalfZ = (kSiPMThickness / 2.0) * mm;
    const G4double sipmGap = 0.05 * mm;

    auto* worldSiPM = new G4Box("SiPMWorld", sipmHalfXY, sipmHalfXY, sipmHalfZ);
    fSiPMLogical = new G4LogicalVolume(worldSiPM, fSiPMMat, "SiPMLogical");
    fSiPMPhys = new G4PVPlacement(
        nullptr,
        G4ThreeVector(0, 0, -halfH - sipmGap - sipmHalfZ),
        fSiPMLogical,
        "SiPM",
        worldLogic,
        false,
        0);

    G4VisAttributes* sipmVis = new G4VisAttributes(G4Colour(0.8,0.8,0.8,1.0));
    sipmVis->SetForceSolid(false);
    fSiPMLogical->SetVisAttributes(sipmVis);

    if (hasCoating) {
        auto* tpbSolid = new G4Tubs("TPB", tpbInnerRadius, rOuter / 2.0, halfH, 0, 360*deg);
        fTpbLogical = new G4LogicalVolume(tpbSolid, fTpbMat, "TPB");
        fTpbPhys = new G4PVPlacement(nullptr, G4ThreeVector(0,0,0),
                                     fTpbLogical, "TPB", worldLogic, false, 0, true);

        auto* tpbSiPM = new G4Box("TPBSiPM", kSiPMSize/2.0 * mm, kSiPMSize/2.0 * mm, 24*um);
        fTpbSiPMLogical = new G4LogicalVolume(tpbSiPM, fTpbMat, "TPBSiPM");
        fTpbSiPMPhys = new G4PVPlacement(nullptr, G4ThreeVector(0, 0, -halfH - kSiPMThickness/2.0 * mm + 24*um/2.0),
                                         fTpbSiPMLogical, "TPBSiPM", worldLogic, false, 0, true);

        G4VisAttributes* tpbVis = new G4VisAttributes(G4Colour(0.95, 0.9, 0.3, 0.35));
        tpbVis->SetForceSolid(true);
        fTpbLogical->SetVisAttributes(tpbVis);
        fTpbSiPMLogical->SetVisAttributes(tpbVis);

        // ── FIX: build the TPB/TPC border surface here, now that both
        // physical volumes actually exist. This only needs a finish/
        // model — the optical constants (RINDEX, WLS...) already live
        // on fTpbMat's own MPT, not on this surface.
        auto* tpbBoundary = new G4OpticalSurface("TPBBoundarySurface");
        tpbBoundary->SetType(dielectric_dielectric);
        tpbBoundary->SetFinish(ground);
        tpbBoundary->SetModel(unified);
        tpbBoundary->SetSigmaAlpha(0.1);

        new G4LogicalBorderSurface("TPB_TPC_Boundary", fTpbPhys, fTpcPhys, tpbBoundary);
        new G4LogicalBorderSurface("TPC_TPB_Boundary", fTpcPhys, fTpbPhys, tpbBoundary);

        new G4LogicalBorderSurface("TPB_SiPM_Boundary", fTpbSiPMPhys, fSiPMPhys, tpbBoundary);
        new G4LogicalBorderSurface("SiPM_TPB_Boundary", fSiPMPhys, fTpbSiPMPhys, tpbBoundary);  

    }

    fWorldPhys = worldPhys;
    SetDicladMode(fDicladMode);   // now runs AFTER fDicladLogical/fTpbPhys/fDicladPhys are all valid

    return worldPhys;
}

void DetectorConstruction::SetOpaqueOpticalProperties(G4Material* material, G4double absorptionLength)
{
    auto* mpt = new G4MaterialPropertiesTable();

    const G4int nEntries = 2;
    G4double photonEnergy[nEntries] = {2.0 * eV, 10.0 * eV};
    G4double rindex[nEntries] = {1.0, 1.0};
    G4double abslen[nEntries] = {absorptionLength, absorptionLength};

    if (material == fXenonGas) {
        rindex[0] = 1.0;
        rindex[1] = 1.0;
        abslen[0] = absorptionLength;
        abslen[1] = absorptionLength;
    }

    mpt->AddProperty("RINDEX", photonEnergy, rindex, nEntries);
    mpt->AddProperty("ABSLENGTH", photonEnergy, abslen, nEntries);
    material->SetMaterialPropertiesTable(mpt);
}

void DetectorConstruction::ConstructSDandField()
{
    auto* sipmSD = new SiPMSD("SiPMSD", "SiPMHitsCollection");
    G4SDManager::GetSDMpointer()->AddNewDetector(sipmSD);
    if (fSiPMLogical) {
        fSiPMLogical->SetSensitiveDetector(sipmSD);
    }
}