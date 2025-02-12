#pragma once
#ifndef LOWSATELLITE_H
#define LOWSATELLITE_H

#include "SatelliteObject.h"

class LowSatellite : public SatelliteObject {
public:
	void Update() override;
};
#endif // !LOWSATELLITE_H
