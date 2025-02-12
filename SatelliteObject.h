#pragma once
#ifndef SATELLITEOBJECT_H
#define SATELLITEOBJECT_H

class SatelliteObject {
protected:
	static double orbital_height;
	static XYZ std_dev;  // m m m
	static double mass;  // kg
	static double rx_power; // dBWt
public:
	SatState state;
	virtual void Update() = 0;
	virtual ~SatelliteObject() = default;
};

#endif // !SATELLITEOBJECT_H
