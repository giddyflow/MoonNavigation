#include "DynamicJam.h"

DynamicJam::DynamicJam(const json& config, std::shared_ptr<Bus> bus, const std::filesystem::path& output_dir) : JamObject(config, bus, output_dir){
	auto& end = config["end"];
	finish_coords.lat = end["lat"];
	finish_coords.lon = end["lon"];
	finish_coords.h = end["h"];

	eventBus = bus;
	eventBus->subscribe("NewStep", [this](std::shared_ptr<Event> eventData) {
		auto newStepData = std::dynamic_pointer_cast<NewStepEvent>(eventData);
		if (newStepData) {
			this->Update(newStepData);
		}
		});
}

void DynamicJam::Update(std::shared_ptr<NewStepEvent> new_step) {
	std::string key = "jam_" + std::to_string(this->state.id);
    json trajectory_point;
    trajectory_point["coords"]["lat"] = state.blh.lat;
    trajectory_point["coords"]["lon"] = state.blh.lon;
    trajectory_point["coords"]["h"] = state.blh.h;
    trajectory_point["timestep"] = new_step->currentTime;
	all_jammers_data[key]["trajectory"].push_back(trajectory_point);
}




