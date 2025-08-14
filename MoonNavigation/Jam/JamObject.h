#pragma once

#include "Service.h"
#include "Engine.h"
#include "Object.h"

struct JamState {
	XYZ ecef;
	BLH blh;
	XYZ velocity;
	int id;
	double power_tx;
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
	JamState state;
public:
	JamObject(const json& config, std::shared_ptr<Bus> bus, const std::filesystem::path& output_dir);
	virtual void PrintInfo() const = 0;
};