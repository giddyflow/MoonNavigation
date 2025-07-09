#pragma once
#ifndef RECEIVEROBJECT_H
#define RECEIVEROBJECT_H

#include "Service.h"
#include "SatelliteObject.h"
#include "JamObject.h"
#include "Engine.h"

struct RecState {
	XYZ ecef;
	XYZ velocity;
	XYZ est_ecef;
	XYZ est_velocity;
	XYZ residual;
	BLH blh;
	DOP dop;
	Clock clock;
	double current_time;
	double est_clock;
	double est_drift;
	bool est;
	int id;
};

class ReceiverEvent : public Event {
public:
	RecState recState;
	ReceiverEvent(RecState state) : recState(state) {}
};

class ReceiverObject : public Object {
protected:
	//void createLogHeader();
	//void log();
	void getVisibleSats(double jams_power);
	void CalcPosition();
	double CalcJamNoise();
	std::vector<XYZ> getSatVelocity();
	std::vector<double> correctionVelo(std::vector<double> vr, std::vector<XYZ> vsatpos, std::vector<XYZ> satpos, XYZ res);
	std::pair<std::vector<double>, std::vector<XYZ>> makeRangeMeasurements();
	std::pair<std::vector<double>, std::vector<XYZ>> makePseudoVeloMeasurements();
	std::tuple<std::vector<double>, std::vector<XYZ>, std::vector<XYZ>> makePseudoRangeMeasurements();
	double CalcSNR(double rxPower, double jamPower);
	double suppress_coeff = 0;
	double mask = 0;
	double snr_treshold = 0;
	double power_spectral_density = 0;
	double clock_instability = 1e-7;
	RecState state;
	std::string nap_key = "";
	std::string nap_data_key = "data_" + nap_key;
	json full_nap_json_data = {};
	void addId();
	void addMetrics();
	void addCoordsDifference();
	void addModelCoords();
	void addEstimatedCoords();

	std::shared_ptr<Bus> eventBus;
	std::vector<SatState> sats;
	std::vector<VisSat> visible_sats;
	std::vector<JamState> jams;
	//std::ofstream logFile;
public:
	ReceiverObject(const json& config, std::shared_ptr<Bus> bus);
	//void Update(std::shared_ptr<NewStepEvent> eventData) override;
	virtual void PrintInfo() const = 0;
	virtual void Calc() = 0;
};
#endif // !RECEIVEROBJECT_H