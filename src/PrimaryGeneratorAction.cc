#include "PrimaryGeneratorAction.hh"

#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4Event.hh"
#include "Randomize.hh"
#include "G4OpticalPhoton.hh"

#include <cmath>

PrimaryGeneratorAction::PrimaryGeneratorAction()
{
    fParticleGun = new G4ParticleGun(1);

    // Default: optical photon source for trajectory visualization.
    G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
    G4ParticleDefinition* opticalPhoton = particleTable->FindParticle("opticalphoton");
    fParticleGun->SetParticleDefinition(opticalPhoton);
    fParticleGun->SetParticleMomentumDirection(G4ThreeVector(0,0,-1));
    fParticleGun->SetParticlePolarization(G4ThreeVector(1,0,0));
    fParticleGun->SetParticleEnergy(fEnergy);
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
    delete fParticleGun;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event)
{
    constexpr G4int nPhotonsPerEvent = 300;
    constexpr G4double holeRadius = 2.25 * mm;  // Radio del agujero (4.5 mm diámetro)

    fParticleGun->SetParticleDefinition(G4OpticalPhoton::OpticalPhotonDefinition());
    fParticleGun->SetParticleEnergy(fEnergy);

    // Generar UN ÚNICO punto de origen aleatorio en el círculo ZX
    // r va de 0 a holeRadius (distribución uniforme en área)
    // θ va de 0 a 2π (ángulo azimutal en plano ZX)
    const G4double r = holeRadius * std::sqrt(G4UniformRand());
    const G4double theta = 2.0 * CLHEP::pi * G4UniformRand();
    
    const G4double xPos = r * std::cos(theta);
    const G4double yPos = r * std::sin(theta);
    const G4double zPos = 0.0 * mm;  // Altura fija

    G4ThreeVector sourcePosition(0, 0, zPos);
    fParticleGun->SetParticlePosition(sourcePosition);

    // Todos los fotones salen del mismo punto con direcciones isótropas
    for (G4int photonIndex = 0; photonIndex < nPhotonsPerEvent; ++photonIndex) {
        const G4double cosTheta = 2.0 * G4UniformRand() - 1.0;
        const G4double sinTheta = std::sqrt(1.0 - cosTheta * cosTheta);
        const G4double phi = 2.0 * CLHEP::pi * G4UniformRand();

        const G4ThreeVector direction(
            sinTheta * std::cos(phi),
            sinTheta * std::sin(phi),
            cosTheta);

        G4ThreeVector refAxis(0.0, 0.0, 1.0);
        if (std::abs(direction.z()) > 0.99) {
            refAxis = G4ThreeVector(0.0, 1.0, 0.0);
        }
        const G4ThreeVector polarization = direction.cross(refAxis).unit();

        fParticleGun->SetParticleMomentumDirection(direction);
        fParticleGun->SetParticlePolarization(polarization);
        fParticleGun->GeneratePrimaryVertex(event);
    }
}

