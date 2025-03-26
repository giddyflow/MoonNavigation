#include "MediumSatellite.h"

MediumSatellite::MediumSatellite(const json& config, std::shared_ptr<Bus> bus)
    : SatelliteObject(config, bus) {
}
