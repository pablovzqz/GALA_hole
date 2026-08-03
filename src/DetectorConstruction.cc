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
#include "G4LogicalBorderSurface.hh"
#include <cmath>

DetectorConstruction::DetectorConstruction() {
    fMessenger = new DetectorMessenger(this);
}
DetectorConstruction::~DetectorConstruction() {
    delete fMessenger;
}

void DetectorConstruction::SetDicladMode(const G4String& mode)
{
    fDicladMode = mode;
    G4LogicalBorderSurface::CleanSurfaceTable();

    if (mode == "reflect") {

        // ── Substrato DICLAD opaco ───────────────────────────────
        auto* dicladMPT = new G4MaterialPropertiesTable();
        std::vector<G4double> energies = {
            6.0*eV,
            7.08*eV,
            10.0*eV
        };
        std::vector<G4double> rindex_diclad  = {1.41, 1.41, 1.41};
        std::vector<G4double> abslen_diclad  = {0.1*mm, 0.1*mm, 0.1*mm};


        // ── Superficie óptica difusa del teflon ─────────────────
        auto* reflectSurface = new G4OpticalSurface("DICLADReflectiveSurface");
        reflectSurface->SetType(dielectric_dielectric);
        reflectSurface->SetFinish(ground);
        reflectSurface->SetModel(unified);
        reflectSurface->SetSigmaAlpha(0.01);

        auto* reflectMPT = new G4MaterialPropertiesTable();
        std::vector<G4double> reflectivity = {0.68, 0.68, 0.68};
        reflectMPT->AddProperty("REFLECTIVITY", energies, reflectivity, true);
        reflectMPT->AddConstProperty("SPECULARSPIKECONSTANT", 0.0,  true);
        reflectMPT->AddConstProperty("SPECULARLOBECONSTANT",  0.0,  true);
        reflectMPT->AddConstProperty("BACKSCATTERCONSTANT",   0.0,  true);
        reflectMPT->AddConstProperty("DIFFUSELOBECONSTANT",   1.0,  true);
        reflectMPT->AddProperty("RINDEX",    energies, rindex_diclad);
        reflectMPT->AddProperty("ABSLENGTH", energies, abslen_diclad);
        reflectSurface->SetMaterialPropertiesTable(reflectMPT);

        if (fTpcPhys && fDicladPhys) {
            new G4LogicalBorderSurface(
                "Xenon_DICLAD_Reflect",
                fTpcPhys,
                fDicladPhys,
                reflectSurface
            );
            new G4LogicalBorderSurface(
                "DICLAD_Xenon_Reflect",
                fDicladPhys,
                fTpcPhys,
                reflectSurface
            );
        }

        G4cout << "[Detector] DICLAD mode: REFLECTIVE (Lambertian teflon, R=0.68)" << G4endl;

    } else if (mode == "TPB" || mode == "tpb") {

        auto* tpbSurface = new G4OpticalSurface("TPBReflectiveSurface");
        tpbSurface->SetType(dielectric_dielectric);
        tpbSurface->SetFinish(ground);
        tpbSurface->SetModel(unified);
        tpbSurface->SetSigmaAlpha(0.01);

        auto* tpbSurfaceMPT = new G4MaterialPropertiesTable();
        std::vector<G4double> energies = {
            6.0*eV,
            7.08*eV,
            10.0*eV
        };

        std::vector<G4double> reflectivity = {0.99, 0.99, 0.99};
        std::vector<G4double> rindex_diclad  = {1.41, 1.41, 1.41};
        std::vector<G4double> abslen_diclad  = {0.1*mm, 0.1*mm, 0.1*mm};

        tpbSurfaceMPT->AddProperty("RINDEX",    energies, rindex_diclad);
        tpbSurfaceMPT->AddProperty("ABSLENGTH", energies, abslen_diclad);

        tpbSurfaceMPT->AddProperty("REFLECTIVITY", energies, reflectivity, true);
        tpbSurfaceMPT->AddConstProperty("SPECULARSPIKECONSTANT", 0.0, true);
        tpbSurfaceMPT->AddConstProperty("SPECULARLOBECONSTANT",  0.0, true);
        tpbSurfaceMPT->AddConstProperty("BACKSCATTERCONSTANT",    0.0, true);
        tpbSurfaceMPT->AddConstProperty("DIFFUSELOBECONSTANT",    1.0, true);
        tpbSurface->SetMaterialPropertiesTable(tpbSurfaceMPT);

        if (fTpbPhys && fDicladPhys) {
            new G4LogicalBorderSurface(
                "TPB_DICLAD_Reflect",
                fTpbPhys,
                fDicladPhys,
                tpbSurface
            );
            new G4LogicalBorderSurface(
                "DICLAD_TPB_Reflect",
                fDicladPhys,
                fTpbPhys,
                tpbSurface
            );
        }

        G4cout << "[Detector] DICLAD mode: TPB coating (WLS in TPB + Lambertian reflectivity at TPB-DICLAD interface)" << G4endl;

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

    auto* tpbMPT = new G4MaterialPropertiesTable();
    std::vector<G4double> rindexEnergies = {
        2.0*eV,
        2.4*eV,
        2.7*eV,
        2.9*eV,
        3.1*eV,
        3.4*eV,
        6.0*eV,
        7.08*eV,
        10.0*eV
    };
    std::vector<G4double> rindex_tpb = {1.67, 1.67, 1.67, 1.67, 1.67, 1.67, 1.67, 1.67, 1.67};
    std::vector<G4double> abslen_tpb = {100.0*m, 100.0*m, 100.0*m, 100.0*m, 100.0*m, 100.0*m, 5.0*nm, 5.0*nm, 5.0*nm};
    std::vector<G4double> wlsAbslen_tpb = {5.0*nm, 5.0*nm, 5.0*nm};
    std::vector<G4double> wlsAbslenEnergy = {6.0*eV, 7.08*eV, 10.0*eV};
    std::vector<G4double> wlsEmissionEnergy = {2.4*eV, 2.7*eV, 2.9*eV, 3.05*eV, 3.2*eV, 3.4*eV};
    std::vector<G4double> wlsEmission = {0.0, 0.15, 0.65, 1.0, 0.55, 0.1};
    tpbMPT->AddProperty("RINDEX", rindexEnergies, rindex_tpb);
    tpbMPT->AddProperty("ABSLENGTH", rindexEnergies, abslen_tpb);
    tpbMPT->AddProperty("WLSABSLENGTH", wlsAbslenEnergy, wlsAbslen_tpb);
    tpbMPT->AddProperty("WLSCOMPONENT", wlsEmissionEnergy, wlsEmission);
    tpbMPT->AddConstProperty("WLSMEANNUMBERPHOTONS", 1.0);
    tpbMPT->AddConstProperty("WLSTIMECONSTANT", 1.68 * ns);
    fTpbMat->SetMaterialPropertiesTable(tpbMPT);

    //
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
    // GALA sits at -Z end (bottom), so drift direction is -Z
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

    if (hasCoating) {
        auto* tpbSolid = new G4Tubs("TPB", tpbInnerRadius, rOuter / 2.0, halfH, 0, 360*deg);
        fTpbLogical = new G4LogicalVolume(tpbSolid, fTpbMat, "TPB");
        fTpbPhys = new G4PVPlacement(nullptr, G4ThreeVector(0,0,0),
                                     fTpbLogical, "TPB", worldLogic, false, 0);

        G4VisAttributes* tpbVis = new G4VisAttributes(G4Colour(0.95, 0.9, 0.3, 0.35));
        tpbVis->SetForceSolid(true);
        fTpbLogical->SetVisAttributes(tpbVis);
    }

    const G4double sipmHalfXY = (kSiPMSize / 2.0) * mm;
    const G4double sipmHalfZ = (kSiPMThickness / 2.0) * mm;
    const G4double sipmGap = 0.05 * mm;

    fWorldPhys = worldPhys;
    SetDicladMode(fDicladMode);

    auto* worldSiPM = new G4Box("SiPMWorld", sipmHalfXY, sipmHalfXY, sipmHalfZ);
    fSiPMLogical = new G4LogicalVolume(worldSiPM, fSiPMMat, "SiPMLogical");
    new G4PVPlacement(
        nullptr,
        G4ThreeVector(0, 0, -halfH - sipmGap - sipmHalfZ),
        fSiPMLogical,
        "SiPM",
        worldLogic,
        false,
        0);

    G4VisAttributes* sipmVis = new G4VisAttributes(G4Colour(0.8,0.8,0.8,1.0));
    sipmVis->SetForceSolid(true);
    fSiPMLogical->SetVisAttributes(sipmVis);

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