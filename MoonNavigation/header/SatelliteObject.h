#pragma once
#ifndef SATELLITEOBJECT_H
#define SATELLITEOBJECT_H

#include "Service.h"
#include "Engine.h"

struct SatState {
	XYZ ecef;
	XYZ velocity;
	XYZ eci;
	XYZ residual;
	BLH blh;
	Ephemeris eph;
	Clock clock;
	double Ptx;
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
	double inclination;
	double start;
	double orbital_height;        // m
	XYZ std_dev;                  // m m m
	double mass;                  // kg
	double tx_power;              // dBWt
	int count = 0;
	double theta0, OMEGA;
	void InitPosition();
	json data = {};
	std::shared_ptr<Bus> eventBus;
	//void Update(std::shared_ptr<NewStepEvent> eventData) override;
public:
	void setStartSeconds(double start_seconds);
	SatelliteObject(const json& config, std::shared_ptr<Bus> bus);
	virtual void PrintInfo() const = 0;
};

#endif // !SATELLITEOBJECT_H
