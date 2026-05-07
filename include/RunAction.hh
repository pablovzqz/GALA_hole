#pragma once

#include "G4UserRunAction.hh"
#include "G4ThreeVector.hh"
#include "globals.hh"

#include <fstream>
#include <vector>

class G4Run;

class RunAction : public G4UserRunAction
{
public:
    RunAction() = default;
    ~RunAction() override = default;

    void BeginOfRunAction(const G4Run*) override;
    void EndOfRunAction(const G4Run*) override;

    void RecordEventSummary(G4int eventID,
                            G4int nPhotons,
                            G4int nDarkCounts,
                            G4int nPE,
                            G4double chargePE,
                            const G4ThreeVector& primaryVertex,
                            const G4ThreeVector* firstSiPMVertex);

private:
    G4long fTotalSiPMPhotons = 0;
    G4long fEventsProcessed = 0;
    G4long fEventsWithSiPMHits = 0;
    std::ofstream fOutput;
};