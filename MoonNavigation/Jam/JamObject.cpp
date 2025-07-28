#include "JamObject.h"

JamObject::JamObject(const json& config, std::shared_ptr<Bus> bus, const std::filesystem::path& output_dir)
    : Object(config)
{
    state.id = this->id;
    state.power_tx = config["power"];
    auto& start = config["coords"];
    state.blh.lat = start["lat"];
    state.blh.lon = start["lon"];
    state.blh.h = start["h"];

}