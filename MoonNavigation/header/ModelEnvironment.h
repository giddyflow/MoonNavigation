#pragma once
#ifndef MODELENVIRONMENT_H
#define MODELENVIRONMENT_H

#include <chrono> // на будущее

#include <memory>
#include <ctime>
#include <sstream>
#include <iomanip>
#include "SatelliteObject.h"
#include "ReceiverObject.h"
#include "JamObject.h"

class ModelEnvironment {
private:
	double start_time = 0;
	double step;
	double stop_time;
	//std::vector<std::shared_ptr<SatelliteObject>> satellites;
	//std::vector<std::shared_ptr<ReceiverObject>> receivers;
	//std::vector<std::shared_ptr<JamObject>> jams;
public:
	ModelEnvironment(const json& config);
	void PrintInfo() { std::cout << "env has been created\n"; }
	double getStopTime(const json& config);
	std::tm parseTime(const std::string& time_str);
	void run();
};
#endif // !MODELENVIRONMENT_H
