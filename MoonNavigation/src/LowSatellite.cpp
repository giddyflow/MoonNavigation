#include "LowSatellite.h"

LowSatellite::LowSatellite(const json& config, std::shared_ptr<Bus> bus)
    : SatelliteObject(config, bus) {
}

