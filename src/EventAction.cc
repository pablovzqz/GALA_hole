#include "EventAction.hh"

#include "RunAction.hh"
#include "PrimaryGeneratorAction.hh"
#include "SiPMSD.hh"

#include "G4Event.hh"
#include "G4PrimaryVertex.hh"
#include "G4SDManager.hh"
#include "G4HCofThisEvent.hh"
#include "G4ThreeVector.hh"
#include "G4UnitsTable.hh"
#include "G4ios.hh"

EventAction::EventAction(RunAction* runAction, PrimaryGeneratorAction* primaryAction)
    : fRunAction(runAction), fPrimaryAction(primaryAction)
{
}

void EventAction::BeginOfEventAction(const G4Event* event)
{
    if (!event) {
        return;
    }
    if (event->GetEventID() % 1000 == 0) {
        G4cout << "### Event " << event->GetEventID() << G4endl;
    }
}

void EventAction::EndOfEventAction(const G4Event* event)
{
    if (!event) {
        return;
    }

    if (fSiPMHCID < 0) {
        fSiPMHCID = G4SDManager::GetSDMpointer()->GetCollectionID("SiPMSD/SiPMHitsCollection");
    }

    // Record generated photons for this event
    if (fRunAction && fPrimaryAction) {
        fRunAction->RecordGeneratedPhotons(event->GetEventID(), fPrimaryAction->GetLastEventPhotons());
    }

    G4int nSiPMPhotons = 0;
    G4ThreeVector firstSiPMVertex(0.0, 0.0, 0.0);
    G4bool hasSiPMInteraction = false;

    auto* hce = event->GetHCofThisEvent();
    if (hce && fSiPMHCID >= 0) {
        auto* hitsCollection = static_cast<SiPMHitsCollection*>(hce->GetHC(fSiPMHCID));
        if (hitsCollection) {
            nSiPMPhotons = hitsCollection->entries();
            if (nSiPMPhotons > 0) {
                firstSiPMVertex = (*hitsCollection)[0]->GetPosition();
                hasSiPMInteraction = true;

                if (fRunAction) {
                    for (G4int i = 0; i < nSiPMPhotons; ++i) {
                        const auto* hit = (*hitsCollection)[i];
                        fRunAction->RecordGeometricHit(
                            event->GetEventID(),
                            i,
                            hit->GetTrackID(),
                            hit->GetTime(),
                            hit->GetPhotonEnergy(),
                            hit->GetPosition(),
                            hit->GetVertexPosition());
                    }
                }
            }
        }
    }

    G4ThreeVector primaryVertex(0.0, 0.0, 0.0);
    auto* primary = event->GetPrimaryVertex(0);
    if (primary) {
        primaryVertex = primary->GetPosition();
    }

    G4cout << "Event " << event->GetEventID()
           << " | geometric hits in SiPM = " << nSiPMPhotons
           << " | primary vertex = ("
           << G4BestUnit(primaryVertex.x(), "Length") << ", "
           << G4BestUnit(primaryVertex.y(), "Length") << ", "
           << G4BestUnit(primaryVertex.z(), "Length") << ")";

    if (hasSiPMInteraction) {
        G4cout << " | first SiPM vertex = ("
               << G4BestUnit(firstSiPMVertex.x(), "Length") << ", "
               << G4BestUnit(firstSiPMVertex.y(), "Length") << ", "
               << G4BestUnit(firstSiPMVertex.z(), "Length") << ")";
    } else {
        G4cout << " | first SiPM vertex = none";
    }

    G4cout << G4endl;
}