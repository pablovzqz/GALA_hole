#pragma once
#include "G4UImessenger.hh"
#include "G4UIcmdWithAString.hh"

class DetectorConstruction;

class DetectorMessenger : public G4UImessenger {
public:
    DetectorMessenger(DetectorConstruction* det);
    ~DetectorMessenger() override;
    void SetNewValue(G4UIcommand* cmd, G4String val) override;

private:
    DetectorConstruction*  fDetector;
    G4UIdirectory*         fDir;
    G4UIcmdWithAString*    fDicladModeCmd;
};