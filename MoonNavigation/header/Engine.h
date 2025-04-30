#pragma once
#ifndef ENGINE_H
#define ENGINE_H

#include "Service.h"

class Event
{
public:
    virtual ~Event() = default;
};

class NewStepEvent : public Event {
public:
    double currentTime;
    NewStepEvent(double time) : currentTime(time) {}
    ~NewStepEvent() {}
};

class StartSecondsEvent : public Event {
public:
    double start_seconds;
    StartSecondsEvent(double seconds) : start_seconds(seconds) {}
    ~StartSecondsEvent() {}
};

class Object {
public:
    virtual void Update(std::shared_ptr<NewStepEvent> eventData) = 0;
    virtual ~Object() = default;
    double current_time;
};

class Bus {
    std::unordered_map<std::string, std::vector<std::function<void(std::shared_ptr<Event>)>>> eventHandlers;
public: 
    void subscribe(const std::string& eventName, std::function<void(std::shared_ptr<Event>)> handler) {
        eventHandlers[eventName].push_back(handler);
    }

    void publish(const std::string& eventName, std::shared_ptr<Event> eventData) {
        if (eventHandlers.count(eventName)) {
            for (auto& handler : eventHandlers[eventName]) {
                handler(eventData); 
            }
        }
    }
};

class Engine {
private:
    std::shared_ptr<Bus> eventBus;
    double start_time = 0;
    double step;
    double stop_time;
    double start_seconds; // переменная для вычисления траектории ECI
public:
    const double getStartSecondsForEci() { return start_seconds; }
    Engine(std::shared_ptr<Bus> bus, json config) {
        eventBus = bus;
        step = config["info"]["step"];
        auto& result = getStopTime(config["info"]);
        stop_time = result.first;
        start_seconds = result.second;
    }

    void run() {
        auto startSecEvent = std::make_shared<StartSecondsEvent>(start_seconds);
        eventBus->publish("StartSeconds", startSecEvent);
        double currentTime = start_time; 
        while (currentTime <= stop_time) {
            std::cout << currentTime << '\n';
            auto newStepData = std::make_shared<NewStepEvent>(currentTime);
            eventBus->publish("NewStep", newStepData);
            eventBus->publish("Calc", nullptr);
            currentTime += step;
        }
    }

};

#endif // !ENGINE_H
