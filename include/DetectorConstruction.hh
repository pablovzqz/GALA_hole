#pragma once
#include "G4VUserDetectorConstruction.hh"
#include "G4Material.hh"
#include "G4LogicalVolume.hh"
#include "globals.hh"
#include <vector>
#include "MessengerDiclad.hh" 

class G4VPhysicalVolume;

class DetectorConstruction : public G4VUserDetectorConstruction
{
public:
    DetectorConstruction();
    ~DetectorConstruction() override;

    G4VPhysicalVolume* Construct() override;
    void ConstructSDandField() override;
    void SetDicladMode(const G4String& mode);  // "absorb" o "reflect"

    static constexpr G4double kGALAThickness  =   5.0; // mm (hole depth)
    static constexpr G4double kHoleDiameter   =   4.5; // mm
    static constexpr G4double kSiPMSize       =   3.0; // mm (3x3 mm²)
    static constexpr G4double kSiPMThickness  =   0.5; // mm

private:
    void DefineMaterials();

    void SetOpaqueOpticalProperties(G4Material* material, G4double absorptionLength);

    G4String fDicladMode = "absorb";           // modo por defecto
    G4VPhysicalVolume* fDicladPhys = nullptr;  // guardar para la superficie
    G4VPhysicalVolume* fTpcPhys    = nullptr;
    G4VPhysicalVolume* fWorldPhys  = nullptr;  // guardar para SetDicladMode
    DetectorMessenger* fMessenger  = nullptr;

    G4Material* fXenonGas   = nullptr;
    G4Material* fDicladMat  = nullptr;
    G4Material* fSiPMMat    = nullptr;
    G4Material* fVacuum     = nullptr;

    G4LogicalVolume* fSiPMLogical = nullptr;

};

