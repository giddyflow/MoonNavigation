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
public:
    Engine(std::shared_ptr<Bus> bus, json config) {
        eventBus = bus;
        step = config["info"]["step"];
        stop_time = getStopTime(config["info"]);
    }

    std::tm parseTime(const std::string& time_str) {
        std::tm tm = {};
        std::istringstream ss(time_str);
        ss >> std::get_time(&tm, "%Y %m %d %H %M %S");
        if (ss.fail()) {
            throw std::runtime_error("Error parsing time: " + time_str);
        }
        return tm;
    }

    double getStopTime(const json& config) {
        std::string start_time_str = config["start_time"];
        std::string stop_time_str = config["stop_time"];
        std::tm start_tm = parseTime(start_time_str);
        std::tm stop_tm = parseTime(stop_time_str);
        std::time_t start = std::mktime(&start_tm);
        std::time_t stop = std::mktime(&stop_tm);
        return difftime(stop, start);
    }

    void run() {
        double currentTime = start_time; 
        while (currentTime <= stop_time) {
            auto newStepData = std::make_shared<NewStepEvent>(currentTime);
            eventBus->publish("NewStep", newStepData);
            eventBus->publish("Calc", nullptr);
            currentTime += step;
        }
    }

};

#endif // !ENGINE_H
