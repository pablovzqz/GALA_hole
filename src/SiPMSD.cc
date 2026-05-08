#include "SiPMSD.hh"

#include "G4Step.hh"
#include "G4HCofThisEvent.hh"
#include "G4SDManager.hh"
#include "G4StepPoint.hh"
#include "G4ThreeVector.hh"
#include "G4Track.hh"
#include "G4OpticalPhoton.hh"

G4ThreadLocal G4Allocator<SiPMHit>* SiPMHitAllocator = nullptr;

SiPMSD::SiPMSD(const G4String& name, const G4String& hitsCollectionName)
    : G4VSensitiveDetector(name)
{
    collectionName.insert(hitsCollectionName);
}

void SiPMSD::Initialize(G4HCofThisEvent* hce)
{
    fHitsCollection = new SiPMHitsCollection(SensitiveDetectorName, collectionName[0]);

    if (fHCID < 0) {
        fHCID = G4SDManager::GetSDMpointer()->GetCollectionID(fHitsCollection);
    }

    hce->AddHitsCollection(fHCID, fHitsCollection);
}

G4bool SiPMSD::ProcessHits(G4Step* step, G4TouchableHistory*)
{
    if (!step) {
        return false;
    }

    auto* track = step->GetTrack();
    if (!track || track->GetDefinition() != G4OpticalPhoton::Definition()) {
        return false;
    }

    auto* hit = new SiPMHit();
    hit->SetTrackID(track->GetTrackID());
    hit->SetTime(step->GetPostStepPoint()->GetGlobalTime());
    hit->SetPhotonEnergy(track->GetTotalEnergy());
    hit->SetPosition(step->GetPostStepPoint()->GetPosition());
    hit->SetVertexPosition(track->GetVertexPosition());
    fHitsCollection->insert(hit);

    track->SetTrackStatus(fStopAndKill);

    return true;
}