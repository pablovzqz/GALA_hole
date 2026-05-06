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
    constexpr G4int nPhotonsPerEvent = 3421*300*0.68;
    constexpr G4double holeRadius = 2.25 * mm;  // Radio del agujero (4.5 mm diámetro)

    // === Parámetros de difusión longitudinal de electrones en Xenón gas ===
    // σ_z = 0.654 mm (difusión espacial)
    // v_d = 0.86 cm/μs (velocidad de deriva)
    // d = 1 cm (distancia de deriva)
    // σ_t = σ_z / v_d ≈ 76 ns (dispersión temporal de llegada)
    constexpr G4double kDiffusionSigmaZ = 0.654 * mm;      // Difusión espacial longitudinal
    constexpr G4double kDriftVelocity = 0.82 * mm / microsecond;  // Velocidad de deriva
    constexpr G4double kDriftDistance = 5.0 * cm;          // Distancia de deriva
    
    // Tiempo de deriva medio
    const G4double kDriftMean = kDriftDistance / kDriftVelocity / 10;  
    
    // Dispersión temporal (σ_t = σ_z / v_d)
    const G4double kDiffusionSigmaT = kDiffusionSigmaZ / kDriftVelocity;  

    fParticleGun->SetParticleDefinition(G4OpticalPhoton::OpticalPhotonDefinition());
    fParticleGun->SetParticleEnergy(fEnergy);

    // Generar UN ÚNICO punto de origen aleatorio en el círculo ZX
    // r va de 0 a holeRadius (distribución uniforme en área)
    // θ va de 0 a 2π (ángulo azimutal en plano ZX)
    const G4double r = holeRadius * std::sqrt(G4UniformRand());
    const G4double theta = 2.0 * CLHEP::pi * G4UniformRand();
    
    const G4double xPos = r * std::cos(theta);
    const G4double yPos = r * std::sin(theta);
    const G4double zPos = 0.5 * mm;  // Altura fija

    G4ThreeVector sourcePosition(0, 0, zPos);
    fParticleGun->SetParticlePosition(sourcePosition);

    // Todos los 300 fotones salen del mismo punto, pero con direcciones isótropas
    // CADA FOTÓN nace con un tiempo inicial que representa la difusión de electrones
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
        
        // === NUEVO: Asignar tiempo inicial según difusión gaussiana ===
        // Los electrones difunden durante la deriva → fotones nacen con distribución temporal
        // t_photon ~ N(t_drift, σ_t) donde σ_t = σ_z / v_d ≈ 76 ns
        G4double photonTime = G4RandGauss::shoot(kDriftMean, kDiffusionSigmaT);
        if (photonTime < 0.0) {
            photonTime = 0.0;  // No permitir tiempos negativos (rarísimo con σ=76ns)
        }
        fParticleGun->SetParticleTime(photonTime);
        
        fParticleGun->GeneratePrimaryVertex(event);
    }
}

