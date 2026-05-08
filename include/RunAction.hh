#pragma once

#include "G4UserRunAction.hh"
#include "G4ThreeVector.hh"
#include "globals.hh"

#include <fstream>
#include <map>

class G4Run;

class RunAction : public G4UserRunAction
{
public:
    RunAction() = default;
    ~RunAction() override = default;

    void BeginOfRunAction(const G4Run*) override;
    void EndOfRunAction(const G4Run*) override;

    void RecordGeometricHit(G4int eventID,
                            G4int hitIndex,
                            G4int trackID,
                            G4double time,
                            G4double photonEnergy,
                            const G4ThreeVector& position,
                            const G4ThreeVector& vertex);

    void RecordGeneratedPhotons(G4int eventID, G4int nPhotons);

private:
    std::ofstream fOutput;
    std::map<G4int, G4int> fGeneratedPhotons;  // event_id -> nPhotons
};