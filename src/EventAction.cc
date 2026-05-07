#include "EventAction.hh"

#include "RunAction.hh"
#include "SiPMSD.hh"

#include "G4Event.hh"
#include "G4PrimaryVertex.hh"
#include "G4SDManager.hh"
#include "G4HCofThisEvent.hh"
#include "G4ThreeVector.hh"
// #include "G4SystemOfUnits.hh"  // not needed here; kept commented by request
#include "G4UnitsTable.hh"
#include "G4ios.hh"
#include "Randomize.hh"

#include <algorithm>
#include <cmath>
#include <vector>

EventAction::EventAction(RunAction* runAction)
    : fRunAction(runAction)
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
            }
        }
    }

    G4ThreeVector primaryVertex(0.0, 0.0, 0.0);
    auto* primary = event->GetPrimaryVertex(0);
    if (primary) {
        primaryVertex = primary->GetPosition();
    }

    // MICROFC-30035-SMT-TR parameters
    constexpr G4double kPDE = 0.30;                    // 30% @ 420 nm
    constexpr G4double kCrossTalkProb = 0.07;          // 7% crosstalk
    constexpr G4double kGainSigmaRel = 0.12;           // 12% gain variation
    constexpr G4int kMicrocells = 4774;                // ~4774 microcells (3x3 mm)
    constexpr G4double kDarkCountRateHz = 860.0e3;    // 860 kHz dark count rate

    // === SiPM PROCESSING: Aplicar modelo global a todos los fotones ===
    const G4int primaryAvalanches = CLHEP::RandBinomial::shoot(nSiPMPhotons, kPDE);
    const G4int primaryAfterSaturation = std::min(primaryAvalanches, kMicrocells);

    const G4int crossTalkAvalanches = CLHEP::RandBinomial::shoot(primaryAfterSaturation, kCrossTalkProb);
    const G4int avalancheAfterPhotonNoise = std::min(primaryAfterSaturation + crossTalkAvalanches, kMicrocells);

    const G4double expectedDarkCounts = kDarkCountRateHz * 1e-6;  
    const G4int darkCounts = CLHEP::RandPoisson::shoot(expectedDarkCounts);

    const G4int totalAvalanches = std::min(avalancheAfterPhotonNoise + darkCounts, kMicrocells);

    const G4double gainSigmaPE = kGainSigmaRel * std::sqrt(static_cast<G4double>(std::max(totalAvalanches, 1)));
    G4double totalChargePE = static_cast<G4double>(totalAvalanches)
                           + G4RandGauss::shoot(0.0, gainSigmaPE);

    if (totalChargePE < 0.0) {
        totalChargePE = 0.0;
    }

    if (fRunAction) {
        fRunAction->RecordEventSummary(
            event->GetEventID(),
            nSiPMPhotons,
            darkCounts,  
            totalAvalanches,
            totalChargePE,
            primaryVertex,
            hasSiPMInteraction ? &firstSiPMVertex : nullptr);
    }

    G4cout << "Event " << event->GetEventID()
           << " | photons in SiPM = " << nSiPMPhotons
           << " | nPE = " << totalAvalanches
           << " | charge(PE) = " << totalChargePE
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