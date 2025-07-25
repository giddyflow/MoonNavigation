#include "LowSatellite.h"

LowSatellite::LowSatellite(const json& config, std::shared_ptr<Bus> bus, const std::filesystem::path& output_dir)
    : SatelliteObject(config, bus, output_dir) {
}

