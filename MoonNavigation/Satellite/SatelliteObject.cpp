#include "SatelliteObject.h"


SatelliteObject::SatelliteObject(const json& config, std::shared_ptr<Bus> bus, const std::filesystem::path& output_dir)
    : Object(config)
{
    orbital_height = config["orbital_height"];
    state.Ptx = config["sat_power"];
    state.clock.shift = config["clock"]["shift"];
    state.clock.drift = config["clock"]["drift"];
    state.clock.instability = config["clock"]["instability"];
    state.id = config["working_points"];

    auto& std = config["std_dev"];
    std_dev.x = std["x"];
    std_dev.y = std["y"];
    std_dev.z = std["z"];

}


void SatelliteObject::setStartSeconds(double start_seconds) {
    this->start_seconds = start_seconds;
}



//void SatelliteObject::Update(std::shared_ptr<NewStepEvent> eventData) {
//	auto newSatData = std::make_shared<SatelliteEvent>(state);
//	eventBus->publish("SatData", newSatData);
//}