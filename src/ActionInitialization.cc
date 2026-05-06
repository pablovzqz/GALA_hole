#include "ActionInitialization.hh"
#include "PrimaryGeneratorAction.hh"
#include "RunAction.hh"
#include "EventAction.hh"
#include "SteppingAction.hh"
#include "DetectorConstruction.hh"
#include "SimMessenger.hh"

ActionInitialization::ActionInitialization(const DetectorConstruction* det)
    : fDetector(det) {}

void ActionInitialization::BuildForMaster() const
{
    SetUserAction(new RunAction());
}

void ActionInitialization::Build() const
{
    auto* primary  = new PrimaryGeneratorAction();
    auto* run      = new RunAction();
    auto* event    = new EventAction(run);
    auto* stepping = new SteppingAction(event, fDetector);

    new SimMessenger(primary, stepping, run);

    SetUserAction(primary);
    SetUserAction(run);
    SetUserAction(event);
    SetUserAction(stepping);
}
