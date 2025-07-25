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
    double orbital_height;
    XYZ std_dev;
    double mass;
    double tx_power;
    int count = 0;
    double theta0, OMEGA;
    std::shared_ptr<Bus> eventBus;

    // Методы для добавления данных, они уже принимают ключ, это отлично
    void addId(const std::string& key);
    void addMetrics(const std::string& key);
    void addCoordsDifference(const std::string& key);
    void addPower(const std::string& key);
    void addModelCoords(const std::string& key);
    void addEstimatedCoords(const std::string& key);
    // Добавим методы для ECI
    void addEciModelCoords(const std::string& key);
    static inline json all_sats_data_ecef = json::object();
    static inline json all_sats_data_eci = json::object();
    static inline bool save_handler_subscribed = false;

public:
    void setStartSeconds(double start_seconds);
    SatelliteObject(const json& config, std::shared_ptr<Bus> bus, const std::filesystem::path& output_dir);
    virtual void PrintInfo() const = 0;

};

#endif // !SATELLITEOBJECT_H
