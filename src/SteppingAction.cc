#include "SteppingAction.hh"

#include "EventAction.hh"
#include "DetectorConstruction.hh"

#include "G4Step.hh"
#include "G4Track.hh"
#include "G4VPhysicalVolume.hh"
#include "G4OpticalPhoton.hh"
#include "G4StepPoint.hh"

SteppingAction::SteppingAction(EventAction* eventAction, const DetectorConstruction* detector)
    : fEventAction(eventAction), fDetector(detector)
{
}

void SteppingAction::UserSteppingAction(const G4Step* step)
{
    if (!step) {
        return;
    }

    auto* track = step->GetTrack();
    if (!track || track->GetDefinition() != G4OpticalPhoton::Definition()) {
        return;
    }

    if (track->GetMomentumDirection().z() > 0.0) {
        track->SetTrackStatus(fStopAndKill);
        return;
    }

    // const auto* postVolume = step->GetPostStepPoint()->GetPhysicalVolume();
    // if (postVolume && postVolume->GetName() == "DICLAD") {
    //     track->SetTrackStatus(fStopAndKill);
    //     return;
    // }
}