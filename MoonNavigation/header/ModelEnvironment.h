#pragma once
#ifndef MODELENVIRONMENT_H
#define MODELENVIRONMENT_H

#include <chrono> // на будущее
#include "Service.h"

class ModelEnvironment {
private:
	double start_time = 0;
	double step;
	double end_time;
public:
	ModelEnvironment(const json& config);
	void PrintInfo() { std::cout << "env has been created\n"; }
};
#endif // !MODELENVIRONMENT_H
