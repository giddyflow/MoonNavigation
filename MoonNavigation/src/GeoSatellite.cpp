#include "GeoSatellite.h"

GeoSatellite::GeoSatellite(const json& config)
    : SatelliteObject(config) {
}

void GeoSatellite::Update() {
    std::cout << "GeoSatellite updating..." << std::endl;
}
