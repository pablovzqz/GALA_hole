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

// Compute longitudinal diffusion (sigma) as a length (same units as `distance`).

G4double sigmaDiffusion(G4double distance)
{
    G4double sigma = std::sqrt(0.0351 * distance / 0.82);
    return sigma;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event)
{
    constexpr G4double holeRadius = 2.25 * mm;  // Radio del agujero (4.5 mm diámetro)
    constexpr G4double driftVelocity = 0.82 * mm / us ;

    fParticleGun->SetParticleDefinition(G4OpticalPhoton::OpticalPhotonDefinition());
    fParticleGun->SetParticleEnergy(fEnergy);

    const G4double vertexDistance = 1 * cm;

    G4double longitudinalDiffusion = sigmaDiffusion(vertexDistance);
    const G4double meanEmissionTime = vertexDistance / driftVelocity;
    const G4double emissionTimeSigma = longitudinalDiffusion / driftVelocity;

    // Gaussiana 2D centrada en el centro del agujero
    G4double sigmaR = 0.8 * mm;
    G4double x, y, r;
    do {
        x = G4RandGauss::shoot(0.0, sigmaR);
        y = G4RandGauss::shoot(0.0, sigmaR);
        r = std::sqrt(x*x + y*y);
    } while (r > holeRadius); // rechaza los que salen del agujero

    const G4double xPos = x;
    const G4double yPos = y;

    G4double electrons = 1886.0;
    G4double photons = 169;
    G4double sigma = std::sqrt(std::max(0.0, fFanoFactor * electrons));
    G4double nElectrons = G4RandGauss::shoot(electrons, sigma);

    const G4double meanPhotons = nElectrons * photons * 0.68;
    G4double nPhotonsPerEvent = CLHEP::RandPoisson::shoot(meanPhotons);

    constexpr G4double A_fit      = 304.19212933;
    constexpr G4double lambda_fit = 0.00119;
    constexpr G4double n_fit      = 4.96810781;
    constexpr G4double z_min      = -2.5  * mm;
    constexpr G4double z_max      = 2.5  * mm;

    // Guardar el número de fotones para este evento
    fLastEventPhotons = (G4int)nPhotonsPerEvent;

    // Cada fotón se emite con su propio z según la distribución asimétrica con shift
    constexpr G4double integral_unnormalized = 1080.687;
    const G4double f_max = A_fit / integral_unnormalized;

    for (G4int photonIndex = 0; photonIndex < nPhotonsPerEvent; ++photonIndex) {
        G4double zPos;
        G4double fz;
        do {
            zPos = z_min + G4UniformRand() * (z_max - z_min);
            fz = A_fit * std::exp(-lambda_fit * std::pow(zPos - z_min, n_fit)) / integral_unnormalized;
        } while (G4UniformRand() > fz / f_max);

        G4ThreeVector sourcePosition(xPos, yPos, zPos);
        fParticleGun->SetParticlePosition(sourcePosition);
        
        G4double emitTime = G4RandGauss::shoot(meanEmissionTime, emissionTimeSigma);
        emitTime = std::max(0.0, emitTime);
        fParticleGun->SetParticleTime(emitTime);

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

