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
    
    // Colectar tiempos de fotones para binning temporal
    std::vector<G4double> photonArrivalTimes;

    auto* hce = event->GetHCofThisEvent();
    if (hce && fSiPMHCID >= 0) {
        auto* hitsCollection = static_cast<SiPMHitsCollection*>(hce->GetHC(fSiPMHCID));
        if (hitsCollection) {
            nSiPMPhotons = hitsCollection->entries();
            if (nSiPMPhotons > 0) {
                firstSiPMVertex = (*hitsCollection)[0]->GetPosition();
                hasSiPMInteraction = true;
                
                // Extraer tiempos de llegada en microsegundos
                for (size_t i = 0; i < hitsCollection->entries(); ++i) {
                    G4double arrivalTimeUs = (*hitsCollection)[i]->GetTime() / 1000.0;
                    photonArrivalTimes.push_back(arrivalTimeUs);
                }
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
    constexpr G4double kBinWidthUs = 1.0;              // 1 μs por bin (en microsegundos)
    constexpr G4int kNumBins = 400;                    // 400 bins × 1 μs = 400 μs total
    constexpr G4double kAcquisitionWindowUs = kBinWidthUs * kNumBins;  // 400 μs

    // === BINNING TEMPORAL: Agrupar fotones por bins de 1 μs ===
    std::vector<G4int> photonsPerBin(kNumBins, 0);
    
    for (G4double arrivalTimeUs : photonArrivalTimes) {
        // Determinar en qué bin cae este fotón
        G4int binIndex = static_cast<G4int>(arrivalTimeUs / kBinWidthUs);
        
        // Contar solo si está dentro del rango [0, 400μs]
        if (binIndex >= 0 && binIndex < kNumBins) {
            photonsPerBin[binIndex]++;
        }
    }

    // === PROCESAMIENTO SiPM POR BIN ===
    // Para cada bin, aplicar el modelo de SiPM independientemente
    G4int totalNPEAllBins = 0;
    G4double totalChargeAllBins = 0.0;
    G4int binWithMaxPhotons = -1;
    G4int maxPhotonsInBin = 0;
    G4double totalDarkCounts = 0.0;

    for (G4int bin = 0; bin < kNumBins; ++bin) {
        G4int nPhotonsInBin = photonsPerBin[bin];
        
        if (nPhotonsInBin > maxPhotonsInBin) {
            maxPhotonsInBin = nPhotonsInBin;
            binWithMaxPhotons = bin;
        }

        const G4int primaryAvalanches = CLHEP::RandBinomial::shoot(nPhotonsInBin, kPDE);
        const G4int primaryAfterSaturation = std::min(primaryAvalanches, kMicrocells);

        const G4int crossTalkAvalanches = CLHEP::RandBinomial::shoot(primaryAfterSaturation, kCrossTalkProb);
        const G4int avalancheAfterPhotonNoise = std::min(primaryAfterSaturation + crossTalkAvalanches, kMicrocells);

        const G4double expectedDarkCounts = kDarkCountRateHz * (kBinWidthUs * 1.0e-6);  // Dark counts para 1 μs
        const G4int darkCounts = CLHEP::RandPoisson::shoot(expectedDarkCounts);

        const G4int totalAvalanches = std::min(avalancheAfterPhotonNoise + darkCounts, kMicrocells);

        const G4double gainSigmaPE = kGainSigmaRel * std::sqrt(static_cast<G4double>(std::max(totalAvalanches, 1)));
        G4double chargePE = static_cast<G4double>(totalAvalanches)
                          + G4RandGauss::shoot(0.0, gainSigmaPE);

        if (chargePE < 0.0) {
            chargePE = 0.0;
        }

        totalNPEAllBins += totalAvalanches;
        totalChargeAllBins += chargePE;
        totalDarkCounts += darkCounts;
    }

    if (fRunAction) {
        fRunAction->RecordEventSummary(
            event->GetEventID(),
            nSiPMPhotons,
            totalDarkCounts,  
            totalNPEAllBins,
            totalChargeAllBins,
            primaryVertex,
            hasSiPMInteraction ? &firstSiPMVertex : nullptr);

        fRunAction->RecordPhotonTimes(event->GetEventID(), photonArrivalTimes);
    }

    // Contar cuántos bins tienen fotones
    G4int binsWithPhotons = 0;
    for (G4int i = 0; i < kNumBins; ++i) {
        if (photonsPerBin[i] > 0) {
            binsWithPhotons++;
        }
    }

    G4cout << "Event " << event->GetEventID()
           << " | photons in SiPM = " << nSiPMPhotons
           << " | bins with photons = " << binsWithPhotons
           << " | max photons in bin = " << maxPhotonsInBin << " (bin " << binWithMaxPhotons << ")"
           << " | total SiPM nPE (400μs) = " << totalNPEAllBins
           << " | total SiPM charge(PE) (400μs) = " << totalChargeAllBins
           << " | primary vertex = ("
           << G4BestUnit(primaryVertex.x(), "Length") << ", "
           << G4BestUnit(primaryVertex.y(), "Length") << ", "
           << G4BestUnit(primaryVertex.z(), "Length") << ")";

    if (hasSiPMInteraction) {
        G4cout << " | first SiPM interaction vertex = ("
               << G4BestUnit(firstSiPMVertex.x(), "Length") << ", "
               << G4BestUnit(firstSiPMVertex.y(), "Length") << ", "
               << G4BestUnit(firstSiPMVertex.z(), "Length") << ")";
    } else {
        G4cout << " | first SiPM interaction vertex = none";
    }

    G4cout << G4endl;
}