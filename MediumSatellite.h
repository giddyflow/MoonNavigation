#pragma once
#ifndef MEDIUMSATELLITE_H
#define MEDIUMSATELLITE_H

#include "SatelliteObject.h"

class MediumSatellite : public SatelliteObject {
public:
	void Update() override;
};
#endif // !MEDIUMSATELLITE_H