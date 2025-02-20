#pragma once
#ifndef SATELLITEOBJECT_H
#define SATELLITEOBJECT_H

#include "Service.h"

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

class SatelliteObject {
protected:
	SatState state;
	double orbital_height; // m
	XYZ std_dev;           // m m m
	double mass;           // kg
	double tx_power;       // dBWt
	void InitPosition();
	int count;
public:
	SatelliteObject(const json& config) {
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
	}
	virtual ~SatelliteObject() = default;
	virtual void PrintInfo() const = 0;
	
};

#endif // !SATELLITEOBJECT_H
