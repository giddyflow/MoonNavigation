#pragma once

#include "Service.h"
#include "RinexTypes.h"

class Event {
public:
    virtual ~Event() = default;
};

class NewStepEvent : public Event {
public:
    double currentTime;
    NewStepEvent(double time) : currentTime(time) {}
};

class StartSecondsEvent : public Event {
public:
    double start_seconds;
    StartSecondsEvent(double seconds) : start_seconds(seconds) {}
};

class ReceiverMeasurementsReadyEvent : public Event {
public:
    double epoch_time;
    RinexEpoch epoch_data; 

    ReceiverMeasurementsReadyEvent(double time, const RinexEpoch& data)
        : epoch_time(time), epoch_data(data) {}
};

class Bus {
    std::unordered_map<std::string, std::vector<std::function<void(std::shared_ptr<Event>)>>> eventHandlers;
public:
    void subscribe(const std::string& eventName, std::function<void(std::shared_ptr<Event>)> handler);
    void publish(const std::string& eventName, std::shared_ptr<Event> eventData);
};