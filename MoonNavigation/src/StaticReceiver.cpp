#include "StaticReceiver.h"

StaticReceiver::StaticReceiver(const json& config, std::shared_ptr<Bus> bus) : ReceiverObject(config, bus){
    eventBus = bus;
    eventBus->subscribe("NewStep", [this](std::shared_ptr<Event> eventData) {
        auto newStepData = std::dynamic_pointer_cast<NewStepEvent>(eventData);
        if (newStepData) {
            this->Update(newStepData); 
        }
        });

    eventBus->subscribe("MedSatData", [this](std::shared_ptr<Event> eventData) {
        auto newMedSatData = std::dynamic_pointer_cast<SatelliteEvent>(eventData);
        if (newMedSatData) {
            sats.push_back(newMedSatData->satState);
        }
        });

    eventBus->subscribe("StatJamData", [this](std::shared_ptr<Event> eventData) {
        auto newStatJamData = std::dynamic_pointer_cast<JamEvent>(eventData);
        if (newStatJamData) {
            jams.push_back(newStatJamData->jamState);
        }
        });

    eventBus->subscribe("Calc", [this](std::shared_ptr<Event> eventData) {
        Calc();
        log();
        });
}

void StaticReceiver::Update(std::shared_ptr<NewStepEvent> new_step) {
    state.clock = markovModelOrder1(state.clock, new_step->currentTime - state.current_time, clock_instability);
    state.current_time = new_step->currentTime;
}

void StaticReceiver::Calc() {
    visible_sats.clear();
    double noise_Prx = CalcJamNoise();
    getVisibleSats(noise_Prx);
    CalcPosition();
    sats.clear();
}


