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

    if (mode == "reflect") {
        // Superficie reflectante en las PAREDES INTERNAS del agujero (DICLAD-Xenon interface)
        // DICLAD actúa como un espejo metálico que refleja fotones ópticos
        SetOpaqueOpticalProperties(fDicladMat, 1.0 * nm);

        // Usar dielectric_metal para reflexión real
        auto* reflectSurface = new G4OpticalSurface("DICLADReflectiveSurface");
        reflectSurface->SetType(dielectric_metal);  // Refleja como un metal
        reflectSurface->SetFinish(polished);         // Reflexión especular pura
        reflectSurface->SetModel(glisur);            // Modelo de reflexión

        auto* reflectMPT = new G4MaterialPropertiesTable();
        std::vector<G4double> e  = {2.0*eV, 10.0*eV};
        std::vector<G4double> r  = {0.68, 0.68};     // 95% reflectividad
        reflectMPT->AddProperty("REFLECTIVITY", e, r);
        reflectSurface->SetMaterialPropertiesTable(reflectMPT);

        // Crear superficie entre TPC (Xenon) y DICLAD - fotones viajan del Xenon al DICLAD
        if (fTpcPhys && fDicladPhys) {
            new G4LogicalBorderSurface("Xenon_DICLAD_Reflect", fTpcPhys, fDicladPhys, reflectSurface);
        }

        G4cout << "[Detector] DICLAD mode: REFLECTIVE (95% metal-like mirror)" << G4endl;

    } else {
        // Modo original: absorción
        SetOpaqueOpticalProperties(fDicladMat, 1.0 * nm);
        G4LogicalBorderSurface::CleanSurfaceTable();

        G4cout << "[Detector] DICLAD mode: ABSORBING (original)" << G4endl;
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

    // Optical properties: hole gas propagates photons, materials absorb them.
    SetOpaqueOpticalProperties(fXenonGas, 1000.0 * m);
    SetOpaqueOpticalProperties(fVacuum,    1000.0 * m);
    SetOpaqueOpticalProperties(fSiPMMat,   1.0 * nm);

    //
}

G4VPhysicalVolume* DetectorConstruction::Construct()
{
    DefineMaterials();

    // ─── World ───────────────────────────────────────────────────
    G4double worldSize = 600.0*mm;
    auto* worldSolid = new G4Box("World", worldSize/2, worldSize/2, worldSize/2);
    auto* worldLogic = new G4LogicalVolume(worldSolid, fVacuum, "World");
    auto* worldPhys  = new G4PVPlacement(nullptr, {}, worldLogic, "World", nullptr, false, 0);
    worldLogic->SetVisAttributes(G4VisAttributes::GetInvisible());

    // ─── TPC Barrel (cylindrical, Xenon gas) ─────────────────────
    // GALA sits at -Z end (bottom), so drift direction is -Z
    G4double rInner = 0;
    G4double rOuter = kHoleDiameter * mm;
    G4double halfH  = kGALAThickness / 2.0 * mm;

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

    auto* tpcSolid  = new G4Tubs("TPC", rInner, rOuter/2, halfH, 0, 360*deg);
    auto* tpcLogic  = new G4LogicalVolume(tpcSolid, fXenonGas, "TPC");
    fTpcPhys = new G4PVPlacement(nullptr, G4ThreeVector(0,0,0), 
                                    tpcLogic, "TPC", worldLogic, false, 0);

    G4VisAttributes* tpcVis = new G4VisAttributes(G4Colour(0.5,0.8,1.0,0.15));
    tpcVis->SetForceSolid(true);
    tpcLogic->SetVisAttributes(tpcVis);

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
