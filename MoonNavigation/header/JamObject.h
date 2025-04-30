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
	double p_tx;
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
	JamObject(const json& config, std::shared_ptr<Bus> bus);
	//void Update(std::shared_ptr<NewStepEvent> eventData) override;
	virtual void PrintInfo() const = 0;
};
#endif // !JAMOBJECT_H