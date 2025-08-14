#pragma once

#include "Service.h"
#include "Engine.h"
#include "Object.h"

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
    double orbital_height;
    XYZ std_dev;
    double mass;
    double tx_power;
    int count = 0;
    double theta0, OMEGA;
    std::shared_ptr<Bus> eventBus;

public:
    void setStartSeconds(double start_seconds);
    SatelliteObject(const json& config, std::shared_ptr<Bus> bus, const std::filesystem::path& output_dir);
    virtual void PrintInfo() const = 0;

};

