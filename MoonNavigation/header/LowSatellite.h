#pragma once
#ifndef LOWSATELLITE_H
#define LOWSATELLITE_H

#include "SatelliteObject.h"

class LowSatellite : public SatelliteObject {
public:
	LowSatellite(const json& config);
	void PrintInfo() const override {
		std::cout << "Low Satellite created" << std::endl;
	}
};
#endif // !LOWSATELLITE_H
