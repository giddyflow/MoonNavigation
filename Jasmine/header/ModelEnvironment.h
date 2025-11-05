#pragma once

#include <chrono> // �� �������

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
public:
	ModelEnvironment(const json& config);
	void PrintInfo() { std::cout << "env has been created\n"; }
	double getStopTime(const json& config);
	std::tm parseTime(const std::string& time_str);
	void run();
};
