#pragma once
#ifndef MEDIUMSATELLITE_H
#define MEDIUMSATELLITE_H

#include "SatelliteObject.h"

class MediumSatellite : public SatelliteObject {
public:
	MediumSatellite(const std::string& type, const json& config) : SatelliteObject(type, config) {

	}
	void Update() override;
};
#endif // !MEDIUMSATELLITE_H