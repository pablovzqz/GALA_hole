#pragma once
#include "G4VUserActionInitialization.hh"

class DetectorConstruction;

class ActionInitialization : public G4VUserActionInitialization
{
public:
    explicit ActionInitialization(const DetectorConstruction* det = nullptr);
    ~ActionInitialization() override = default;

    void BuildForMaster() const override;
    void Build() const override;

private:
    const DetectorConstruction* fDetector = nullptr;
};

