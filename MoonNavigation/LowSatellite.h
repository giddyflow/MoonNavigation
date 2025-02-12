#pragma once
#ifndef LOWSATELLITE_H
#define LOWSATELLITE_H

#include "SatelliteObject.h"

class LowSatellite : public SatelliteObject {
public:
	LowSatellite(const std::string& type, const json& config) : SatelliteObject(type, config) {
	
	}
	void Update() override;
};
#endif // !LOWSATELLITE_H
