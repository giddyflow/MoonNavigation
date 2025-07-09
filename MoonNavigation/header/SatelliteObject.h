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
	double start_seconds;
	double orbital_height;        // m
	XYZ std_dev;                  // m m m
	double mass;                  // kg
	double tx_power;              // dBWt
	int count = 0;
	double theta0, OMEGA;
	void InitPosition();
	std::string nka_key = "";
	std::string nka_data_key = "data_" + nka_key;
	json full_sat_json_data = {};
	json full_sat_json_data_eci = {};
	std::shared_ptr<Bus> eventBus;

	void addId();
	void addCoordsDifference();
	void addPower();
	void addEstimatedCoords();
	void addMetrics();
	void addModelCoords();
	//void Update(std::shared_ptr<NewStepEvent> eventData) override;
public:
	void setStartSeconds(double start_seconds);
	SatelliteObject(const json& config, std::shared_ptr<Bus> bus);
	virtual void PrintInfo() const = 0;
};

#endif // !SATELLITEOBJECT_H
