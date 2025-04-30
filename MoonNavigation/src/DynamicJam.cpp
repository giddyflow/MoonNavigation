#include "DynamicJam.h"

DynamicJam::DynamicJam(const json& config, std::shared_ptr<Bus> bus) : JamObject(config, bus){
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
	int a = 5;
}




