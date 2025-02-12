#pragma once
#ifndef GEOSATELLITE_H
#define GEOSATELLITE_H

#include "SatelliteObject.h"

class GeoSatellite : public SatelliteObject {
public:
	GeoSatellite(const std::string& type, const json& config) : SatelliteObject(type, config) {

	}
	void Update() override;
};
#endif // !GEOSATELLITE_H