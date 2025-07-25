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
	static inline json all_jammers_data = json::object();
    static inline bool save_handler_subscribed = false;
public:
	JamObject(const json& config, std::shared_ptr<Bus> bus, const std::filesystem::path& output_dir);
	virtual void PrintInfo() const = 0;
};
#endif // !JAMOBJECT_H