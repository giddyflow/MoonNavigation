#include "../header/Engine.h"
#include <iostream>
#include <utility> 

Engine::Engine(std::shared_ptr<Bus> bus, json config) {
    this->eventBus = bus;

    this->step = config.value("period", 1.0);
    
    const auto& result = getStopTime(config);
    this->stop_time = result.first;
    this->start_seconds = result.second;
    this->start_time = 0.0;
}

void Engine::run() {
    auto startSecEvent = std::make_shared<StartSecondsEvent>(start_seconds);
    eventBus->publish("StartSeconds", startSecEvent);

    double currentTime = start_time; 

    while (currentTime <= stop_time) {
        std::cout << "Engine Tick: " << currentTime << "s" << std::endl;

        auto newStepData = std::make_shared<NewStepEvent>(currentTime);
        eventBus->publish("NewStep", newStepData);
        eventBus->publish("Calc", nullptr);
        eventBus->publish("AccessibilityStep", newStepData);
        currentTime += step;
    }

    std::cout << "Modeling finished. Publishing SaveAllResults event." << std::endl;
    eventBus->publish("SaveAllResults", nullptr);
}