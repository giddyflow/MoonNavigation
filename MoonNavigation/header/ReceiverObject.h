#pragma once
#ifndef RECEIVEROBJECT_H
#define RECEIVEROBJECT_H

#include "Service.h"
#include "Engine.h"

struct RecState {
	XYZ ecef;
	BLH blh;
	XYZ velocity;
	Clock clock;
	int id;
	double current_time;
};

class ReceiverEvent : public Event {
public:
	RecState recState;
	ReceiverEvent(RecState state) : recState(state) {}
};

class ReceiverObject : public Object {
protected:
	RecState state;
	double suppress_coeff = 0;
	double mask = 0;
	double snr_treshold = 0;
	double power_spectral_density = 0;
	std::shared_ptr<Bus> eventBus;
public:
	ReceiverObject(const json& config, std::shared_ptr<Bus> bus) {
		suppress_coeff = config["suppress_coeff"];
		mask = config["mask"];
		snr_treshold = config["snr_treshold"];
		power_spectral_density = config["power_spectral_density"];
		state.id = config["id"];
		auto& start = config["start"];
		state.blh.lat = start["lat"];
		state.blh.lon = start["lon"];
		state.blh.h = start["h"];
		auto& clock = config["clock"];
		state.clock.drift = clock["drift"];
		state.clock.shift = clock["shift"];

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
		auto newRecData = std::make_shared<ReceiverEvent>(state);
		eventBus->publish("RecData", newRecData);
	}
	virtual void PrintInfo() const = 0;
};
#endif // !RECEIVEROBJECT_H