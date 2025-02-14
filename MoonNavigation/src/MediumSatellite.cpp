#include "MediumSatellite.h"

MediumSatellite::MediumSatellite(const json& config)
    : SatelliteObject(config) {
}

void MediumSatellite::Update() {
    std::cout << "MediumSatellite updating..." << std::endl;
}
