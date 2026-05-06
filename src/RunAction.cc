#include "RunAction.hh"

#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4ios.hh"

void RunAction::BeginOfRunAction(const G4Run* run)
{
	fTotalSiPMPhotons = 0;
	fEventsProcessed = 0;
	fEventsWithSiPMHits = 0;
	fOutput.open("sipm_events.txt");
	fOutput << "event_id\tphotons_in_SiPM\tdark_counts\tsipm_nPE\tsipm_charge_pe\tprimary_x\tprimary_y\tprimary_z\tfirst_sipm_x\tfirst_sipm_y\tfirst_sipm_z\n";
	
	// Nuevo: Archivo para tiempos de fotones
	fPhotonTimesOutput.open("photon_times.txt");
	fPhotonTimesOutput << "event_id\tphoton_time_us\n";

	G4RunManager::GetRunManager()->SetPrintProgress(100);
	G4cout << "### Run " << run->GetRunID() << " start" << G4endl;
}

void RunAction::RecordEventSummary(G4int eventID,
                                   G4int nPhotons,
							   G4int nDarkCounts,
								   G4int nPE,
								   G4double chargePE,
                                   const G4ThreeVector& primaryVertex,
                                   const G4ThreeVector* firstSiPMVertex)
{
	++fEventsProcessed;
	fTotalSiPMPhotons += nPhotons;
	if (nPhotons > 0) {
		++fEventsWithSiPMHits;
	}

	if (fOutput.is_open()) {
		fOutput << eventID << '\t'
		        << nPhotons << '\t'
		        << nDarkCounts << '\t'
		        << nPE << '\t'
		        << chargePE << '\t'
		        << primaryVertex.x() / mm << '\t'
		        << primaryVertex.y() / mm << '\t'
		        << primaryVertex.z() / mm << '\t';

		if (firstSiPMVertex) {
			fOutput << firstSiPMVertex->x() / mm << '\t'
			        << firstSiPMVertex->y() / mm << '\t'
			        << firstSiPMVertex->z() / mm;
		} else {
			fOutput << "none\tnone\tnone";
		}

		fOutput << '\n';
	}
}

void RunAction::EndOfRunAction(const G4Run* run)
{
	if (fOutput.is_open()) {
		fOutput.close();
	}
	if (fPhotonTimesOutput.is_open()) {
		fPhotonTimesOutput.close();
	}

	G4cout << "### SiPM summary: total photons = " << fTotalSiPMPhotons
	       << ", events = " << fEventsProcessed
	       << ", events with hits = " << fEventsWithSiPMHits << G4endl;
	G4cout << "### Run " << run->GetRunID() << " end" << G4endl;
}

void RunAction::RecordPhotonTimes(G4int eventID,
                                  const std::vector<G4double>& photonArrivalTimes)
{
	if (fPhotonTimesOutput.is_open()) {
		for (G4double time : photonArrivalTimes) {
			// Convertir tiempo de ns a μs para mejor legibilidad
			fPhotonTimesOutput << eventID << '\t' << (time / 1000.0) << '\n';
		}
	}
}