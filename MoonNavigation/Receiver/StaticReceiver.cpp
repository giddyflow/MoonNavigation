#include "StaticReceiver.h"

StaticReceiver::StaticReceiver(const json& config, std::shared_ptr<Bus> bus, const std::filesystem::path& output_dir) 
    : ReceiverObject(config, bus, output_dir){
    eventBus = bus;
    eventBus->subscribe("NewStep", [this](std::shared_ptr<Event> eventData) {
        if (auto newStepData = std::dynamic_pointer_cast<NewStepEvent>(eventData)) {
            this->Update(newStepData); 
        }
    });

    eventBus->subscribe("MedSatData", [this](std::shared_ptr<Event> eventData) {
        if (auto newMedSatData = std::dynamic_pointer_cast<SatelliteEvent>(eventData)) {
            sats.push_back(newMedSatData->satState);
        }
    });

    eventBus->subscribe("StatJamData", [this](std::shared_ptr<Event> eventData) {
        if (auto newStatJamData = std::dynamic_pointer_cast<JamEvent>(eventData)) {
            jams.push_back(newStatJamData->jamState);
        }
    });

    eventBus->subscribe("Calc", [this](std::shared_ptr<Event> eventData) {
        Calc();
    });
}

void StaticReceiver::Update(std::shared_ptr<NewStepEvent> new_step) {
    double dt = new_step->currentTime - state.current_time;
    state.clock = markovModelOrder1(state.clock, dt, clock_instability);
    state.current_time = new_step->currentTime;
}

void StaticReceiver::Calc() {
    visible_sats.clear();
    double noise_Prx = CalcJamNoise();
    getVisibleSats(noise_Prx);
    CalcPosition(); 
    sats.clear();

    auto receiverStateEvent = std::make_shared<ReceiverEvent>(state);
    eventBus->publish("ReceiverData", receiverStateEvent);
}


