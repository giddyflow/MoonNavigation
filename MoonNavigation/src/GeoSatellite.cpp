#include "GeoSatellite.h"

GeoSatellite::GeoSatellite(const json& config, std::shared_ptr<Bus> bus)
    : SatelliteObject(config, bus){
}



