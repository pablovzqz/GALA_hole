#pragma once

class PrimaryGeneratorAction;
class SteppingAction;
class RunAction;

class SimMessenger
{
public:
    SimMessenger(PrimaryGeneratorAction*, SteppingAction*, RunAction*) {}
    ~SimMessenger() = default;
};