#pragma once

#include "G4UserSteppingAction.hh"

class EventAction;
class DetectorConstruction;
class G4Step;

class SteppingAction : public G4UserSteppingAction
{
public:
    SteppingAction(EventAction* eventAction = nullptr, const DetectorConstruction* detector = nullptr);
    ~SteppingAction() override = default;

    void UserSteppingAction(const G4Step*) override;

private:
    EventAction* fEventAction = nullptr;
    const DetectorConstruction* fDetector = nullptr;
};