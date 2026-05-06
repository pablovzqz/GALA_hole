#pragma once

#include "G4UserEventAction.hh"
#include "globals.hh"

class RunAction;
class G4Event;

class EventAction : public G4UserEventAction
{
public:
    explicit EventAction(RunAction* runAction = nullptr);
    ~EventAction() override = default;

    void BeginOfEventAction(const G4Event*) override;
    void EndOfEventAction(const G4Event*) override;

private:
    RunAction* fRunAction = nullptr;
    G4int fSiPMHCID = -1;
};