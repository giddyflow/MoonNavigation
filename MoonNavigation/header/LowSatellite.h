#pragma once
#ifndef LOWSATELLITE_H
#define LOWSATELLITE_H

#include "SatelliteObject.h"

class LowSatellite : public SatelliteObject {
public:
	LowSatellite(const json& config);
	void PrintInfo() const override {
		std::cout << "low Satellite created with id " << state.id << '\n';
	}
};
#endif // !LOWSATELLITE_H
