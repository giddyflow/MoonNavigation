#pragma once
#ifndef RECEIVEROBJECT_H
#define RECEIVEROBJECT_H

#include "Service.h"

struct RecState {
	XYZ ecef;
	BLH blh;
	XYZ velocity;
	Clock clock;
	int id;
	double current_time;
};

class ReceiverObject{
protected:
	RecState state;
	double suppress_coeff = 0;
	double mask = 0;
	double snr_treshold = 0;
	double power_spectral_density = 0;
public:
	ReceiverObject(const json& config) {
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
	}
	virtual ~ReceiverObject() = default;
	virtual void Update() = 0;
	virtual void CalcPosition() = 0;
	virtual void PrintInfo() const = 0;
};
#endif // !RECEIVEROBJECT_H