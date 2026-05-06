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


private:
    G4ParticleGun* fParticleGun = nullptr;
    G4double       fEnergy        = 7.21 * CLHEP::eV;

};

