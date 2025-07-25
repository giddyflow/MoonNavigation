#include "GeoSatellite.h"

GeoSatellite::GeoSatellite(const json& config, std::shared_ptr<Bus> bus, const std::filesystem::path& output_dir)
    : SatelliteObject(config, bus, output_dir){
}



