#pragma once
#ifndef GEOSATELLITE_H
#define GEOSATELLITE_H

#include "SatelliteObject.h"

class GeoSatellite : public SatelliteObject {
public:
	GeoSatellite(const json& config) : SatelliteObject(config) {

	}
	void PrintInfo() const override {
		std::cout << "geo Satellite created" << std::endl;
	}
};
#endif // !GEOSATELLITE_H