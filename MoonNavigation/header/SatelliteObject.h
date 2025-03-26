#pragma once
#ifndef SATELLITEOBJECT_H
#define SATELLITEOBJECT_H

#include "Service.h"
#include "Engine.h"

struct SatState {
	XYZ ecef;
	XYZ eci;
	BLH blh;
	Clock clock;
	double rx_power;
	bool visible;
	int id;
	double current_time;
};

class SatelliteEvent : public Event {
public:
	SatState satState;
	SatelliteEvent(SatState state) : satState(state) {}
};

class SatelliteObject : public Object {
protected:
	SatState state;
	double orbital_height; // m
	XYZ std_dev;           // m m m
	double mass;           // kg
	double tx_power;       // dBWt
	void InitPosition();
	int count = 0;
	std::shared_ptr<Bus> eventBus;
public:
	SatelliteObject(const json& config, std::shared_ptr<Bus> bus) {
		mass = config["mass"];
		orbital_height = config["orbital_height"];
		tx_power = config["tx_power"];
		state.clock.shift = config["clock"]["shift"];
		state.clock.drift = config["clock"]["drift"];
		state.clock.instability = config["clock"]["instability"];
		state.id = config["id"];
		auto& std = config["std_dev"];
		std_dev.x = std["x"];
		std_dev.y = std["y"];
		std_dev.z = std["z"];

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
		auto newSatData = std::make_shared<SatelliteEvent>(state);
		eventBus->publish("SatData", newSatData);
	}
	virtual void PrintInfo() const = 0;
};

#endif // !SATELLITEOBJECT_H
