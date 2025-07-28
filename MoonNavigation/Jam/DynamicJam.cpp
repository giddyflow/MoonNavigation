#include "DynamicJam.h"

DynamicJam::DynamicJam(const json& config, std::shared_ptr<Bus> bus, const std::filesystem::path& output_dir) 
    : JamObject(config, bus, output_dir){
	auto& end = config["end"];
	finish_coords.lat = end["lat"];
	finish_coords.lon = end["lon"];
	finish_coords.h = end["h"];

	eventBus = bus;
	eventBus->subscribe("NewStep", [this](std::shared_ptr<Event> eventData) {
		if (auto newStepData = std::dynamic_pointer_cast<NewStepEvent>(eventData)) {
			this->Update(newStepData);
		}
	});
}

void DynamicJam::Update(std::shared_ptr<NewStepEvent> new_step) {
    state.current_time = new_step->currentTime;
    auto newJamData = std::make_shared<JamEvent>(state);
    eventBus->publish("JamData", newJamData);
}



