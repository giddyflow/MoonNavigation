#pragma once
#ifndef GEOSATELLITE_H
#define GEOSATELLITE_H

#include "SatelliteObject.h"

class GeoSatellite : public SatelliteObject {
public:
	GeoSatellite(const json& config);
	void PrintInfo() const override {
		std::cout << "geo Satellite created with id " << state.id << '\n';
	}
};
#endif // !GEOSATELLITE_H