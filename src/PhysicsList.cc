#include "PhysicsList.hh"

#include "G4EmStandardPhysics.hh"
#include "G4OpticalPhysics.hh"
#include "G4DecayPhysics.hh"
#include "G4RadioactiveDecayPhysics.hh"
#include "G4SystemOfUnits.hh"
#include "G4OpticalParameters.hh"

// Particles
#include "G4Gamma.hh"
#include "G4Electron.hh"
#include "G4Positron.hh"
#include "G4OpticalPhoton.hh"
#include "G4Geantino.hh"

PhysicsList::PhysicsList()
{
    SetVerboseLevel(0);

    // Standard EM physics (handles ionization, photoelectric, Compton, etc.)
    RegisterPhysics(new G4EmStandardPhysics(0));

    // Decays
    RegisterPhysics(new G4DecayPhysics(0));

    // Optical photon physics (needed for VUV photon tracking)
    auto* opticalPhysics = new G4OpticalPhysics(0);
    RegisterPhysics(opticalPhysics);

    // Configure optical parameters
    auto* optParams = G4OpticalParameters::Instance();
    optParams->SetProcessActivation("Scintillation", false); // we handle emission manually
    optParams->SetProcessActivation("Cerenkov",      false);
    optParams->SetProcessActivation("OpAbsorption",  true);
    optParams->SetProcessActivation("OpBoundary",    true);
    optParams->SetProcessActivation("OpRayleigh",    false);
}

PhysicsList::~PhysicsList() {}

void PhysicsList::ConstructParticle()
{
    G4VModularPhysicsList::ConstructParticle();
    G4Gamma::GammaDefinition();
    G4Electron::ElectronDefinition();
    G4Positron::PositronDefinition();
    G4OpticalPhoton::OpticalPhotonDefinition();
    G4Geantino::GeantinoDefinition();
}

void PhysicsList::ConstructProcess()
{
    G4VModularPhysicsList::ConstructProcess();
}
