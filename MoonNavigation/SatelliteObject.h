#pragma once
#ifndef SATELLITEOBJECT_H
#define SATELLITEOBJECT_H

#include "Service.h"

struct SatState {
	XYZ ecef;
	XYZ eci;
	BLH blh;
	double rx_power;
	bool visible;
	int id;
};

class SatelliteObject {
protected:
	static const double orbital_height;
	static XYZ std_dev;  // m m m
	static double mass;  // kg
	static double rx_power; // dBWt
public:
	SatState state;
	SatelliteObject(const std::string& type, const json& config) {

	}
	virtual void Update() = 0;
	virtual ~SatelliteObject() = default;
};

#endif // !SATELLITEOBJECT_H
