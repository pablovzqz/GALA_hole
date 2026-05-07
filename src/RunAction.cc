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

	G4cout << "### SiPM summary: total photons = " << fTotalSiPMPhotons
	       << ", events = " << fEventsProcessed
	       << ", events with hits = " << fEventsWithSiPMHits << G4endl;
	G4cout << "### Run " << run->GetRunID() << " end" << G4endl;
}
