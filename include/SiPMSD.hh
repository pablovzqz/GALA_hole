#pragma once

#include "G4Allocator.hh"
#include "G4THitsCollection.hh"
#include "G4ThreeVector.hh"
#include "G4VHit.hh"
#include "G4VSensitiveDetector.hh"
#include "globals.hh"

class G4Step;
class G4TouchableHistory;

class SiPMHit : public G4VHit
{
public:
    SiPMHit() = default;
    ~SiPMHit() override = default;

    SiPMHit(const SiPMHit&) = default;
    SiPMHit& operator=(const SiPMHit&) = default;

    void* operator new(size_t);
    void operator delete(void* hit);

    void SetPosition(const G4ThreeVector& position) { fPosition = position; }
    void SetVertexPosition(const G4ThreeVector& vertex) { fVertexPosition = vertex; }
    void SetTime(G4double time) { fTime = time; }
    void SetPhotonEnergy(G4double energy) { fPhotonEnergy = energy; }
    void SetTrackID(G4int trackID) { fTrackID = trackID; }

    const G4ThreeVector& GetPosition() const { return fPosition; }
    const G4ThreeVector& GetVertexPosition() const { return fVertexPosition; }
    G4double GetTime() const { return fTime; }
    G4double GetPhotonEnergy() const { return fPhotonEnergy; }
    G4int GetTrackID() const { return fTrackID; }

private:
    G4ThreeVector fPosition;
    G4ThreeVector fVertexPosition;
    G4double fTime = 0.0;
    G4double fPhotonEnergy = 0.0;
    G4int fTrackID = -1;
};

using SiPMHitsCollection = G4THitsCollection<SiPMHit>;

extern G4ThreadLocal G4Allocator<SiPMHit>* SiPMHitAllocator;

class SiPMSD : public G4VSensitiveDetector
{
public:
    SiPMSD(const G4String& name, const G4String& hitsCollectionName);
    ~SiPMSD() override = default;

    void Initialize(G4HCofThisEvent*) override;
    G4bool ProcessHits(G4Step*, G4TouchableHistory*) override;

private:
    SiPMHitsCollection* fHitsCollection = nullptr;
    G4int fHCID = -1;
};

inline void* SiPMHit::operator new(size_t)
{
    if (!SiPMHitAllocator) {
        SiPMHitAllocator = new G4Allocator<SiPMHit>;
    }
    return SiPMHitAllocator->MallocSingle();
}

inline void SiPMHit::operator delete(void* hit)
{
    SiPMHitAllocator->FreeSingle(static_cast<SiPMHit*>(hit));
}