#pragma once
#ifndef JAMOBJECT_H
#define JAMOBJECT_H

#include "Service.h"
#include "Engine.h"

struct JamState {
	XYZ ecef;
	BLH blh;
	XYZ velocity;
	int id;
	double current_time = 0;
};

class JamEvent : public Event {
public:
	JamState jamState;
	JamEvent(JamState state) : jamState(state) {}
};


class JamObject : public Object{
protected:
	std::shared_ptr<Bus> eventBus;
	std::shared_ptr<json> config;
	JamState state;
	double tx_power;
public:
	JamObject(const json& config, std::shared_ptr<Bus> bus){
		tx_power = config["tx_power"];
		auto& start = config["start"];
		state.blh.lat = start["lat"];
		state.blh.lon = start["lon"];
		state.blh.h = start["h"];
		eventBus = bus;

		eventBus->subscribe("NewStep", [this](std::shared_ptr<Event> eventData) {
			//std::cout << "Received event: " << typeid(*eventData).name() << std::endl;
			auto newStepData = std::dynamic_pointer_cast<NewStepEvent>(eventData);
			if (newStepData) {
				this->Update(newStepData);
			}
		});

	}
	void Update(std::shared_ptr<NewStepEvent> eventData) override {
		auto newJamData = std::make_shared<JamEvent>(state);
		eventBus->publish("JamData", newJamData);
	}
	virtual void PrintInfo() const = 0;
};
#endif // !JAMOBJECT_H