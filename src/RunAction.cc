#include "RunAction.hh"

#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4ios.hh"

void RunAction::BeginOfRunAction(const G4Run* run)
{
	fOutput.open("geometric_hits_absorbentGALA_newPITCH.txt");
	fOutput << "event_id\ttime_ns\tvx_mm\tvy_mm\tvz_mm\tnPhotons\n";

	G4RunManager::GetRunManager()->SetPrintProgress(100);
	G4cout << "### Run " << run->GetRunID() << " start" << G4endl;
}

void RunAction::RecordGeometricHit(G4int eventID,
                                   G4int hitIndex,
                                   G4int trackID,
                                   G4double time,
                                   G4double photonEnergy,
                                   const G4ThreeVector& position,
                                   const G4ThreeVector& vertex)
{
	if (fOutput.is_open()) {
		G4int nPhotons = fGeneratedPhotons[eventID];
		fOutput << eventID << '\t'
		        << time / ns << '\t'
		        << vertex.x() / mm << '\t'
		        << vertex.y() / mm << '\t'
		        << vertex.z() / mm << '\t'
		        << nPhotons;

		fOutput << '\n';
	}
}

void RunAction::RecordGeneratedPhotons(G4int eventID, G4int nPhotons)
{
	fGeneratedPhotons[eventID] = nPhotons;
}

void RunAction::EndOfRunAction(const G4Run* run)
{
	if (fOutput.is_open()) {
		fOutput.close();
	}

	G4cout << "### Run " << run->GetRunID() << " end" << G4endl;
}
