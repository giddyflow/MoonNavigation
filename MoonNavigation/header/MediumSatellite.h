#pragma once
#ifndef MEDIUMSATELLITE_H
#define MEDIUMSATELLITE_H

#include "SatelliteObject.h"

class MediumSatellite : public SatelliteObject {
public:
	MediumSatellite(const json& config);
	void PrintInfo() const override {
		std::cout << "medium Satellite created" << std::endl;
	}
};
#endif // !MEDIUMSATELLITE_H