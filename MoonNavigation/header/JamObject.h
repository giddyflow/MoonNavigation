#pragma once
#ifndef JAMOBJECT_H
#define JAMOBJECT_H

#include "Service.h"

struct JamState {
	XYZ ecef;
	BLH blh;
	XYZ velocity;
	int id = 0;
	double current_time = 0;
};

class JamObject {
protected:
	JamState state;
	double tx_power;
public:
	JamObject(const json& config) {
		std::cout << config.dump(4) << '\n';
		tx_power = config["tx_power"];
		auto start = config["start"];
		state.blh.lat = start["lat"];
		state.blh.lon = start["lon"];
		state.blh.h = start["h"];
	}
	virtual ~JamObject() = default;
	virtual void Update() = 0;
	virtual void PrintInfo() const = 0;
};
#endif // !JAMOBJECT_H