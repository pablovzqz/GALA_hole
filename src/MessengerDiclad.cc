#include "MessengerDiclad.hh"
#include "DetectorConstruction.hh"
#include "G4UIdirectory.hh"

DetectorMessenger::DetectorMessenger(DetectorConstruction* det)
    : fDetector(det)
{
    fDir = new G4UIdirectory("/detector/");
    fDir->SetGuidance("Detector control");

    fDicladModeCmd = new G4UIcmdWithAString("/detector/dicladMode", this);
    fDicladModeCmd->SetGuidance("Set DICLAD optical mode: 'absorb' or 'reflect'");
    fDicladModeCmd->SetParameterName("mode", false);
    fDicladModeCmd->SetCandidates("absorb reflect");
    fDicladModeCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
}

DetectorMessenger::~DetectorMessenger() {
    delete fDicladModeCmd;
    delete fDir;
}

void DetectorMessenger::SetNewValue(G4UIcommand* cmd, G4String val) {
    if (cmd == fDicladModeCmd) {
        fDetector->SetDicladMode(val);
    }
}