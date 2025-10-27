#pragma once

#include "EventBus.h"

std::pair<double, double> getStopTime(const json& config);

class Engine {
private:
    std::shared_ptr<Bus> eventBus;
    double start_time = 0;
    double step;
    double stop_time;
    double start_seconds;
public:
    Engine(std::shared_ptr<Bus> bus, json config);
    
    void run();

    double getStartSecondsForEci() { return start_seconds; }
};