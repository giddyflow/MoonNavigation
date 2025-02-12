#pragma once
#ifndef GEOSATELLITE_H
#define GEOSATELLITE_H

#include "SatelliteObject.h"

class GeoSatellite : public SatelliteObject {
public:
	void Update() override;
};
#endif // !GEOSATELLITE_H