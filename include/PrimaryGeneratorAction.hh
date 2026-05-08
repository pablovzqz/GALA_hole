#pragma once
#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"
#include "globals.hh"

class G4Event;

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
public:
    PrimaryGeneratorAction();
    ~PrimaryGeneratorAction() override;

    void GeneratePrimaries(G4Event* event) override;

    void SetEventEnergy(G4double e)   { fEnergy = e; }
    void SetFanoFactor(G4double f)     { fFanoFactor = f; }
    G4int GetLastEventPhotons() const { return fLastEventPhotons; }


private:
    G4ParticleGun* fParticleGun = nullptr;
    G4double       fEnergy        = 7.21 * CLHEP::eV;
    G4double       fFanoFactor    = 0.15;
    G4int          fLastEventPhotons = 0;

};

