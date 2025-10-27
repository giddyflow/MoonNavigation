#include "StaticJam.h"

StaticJam::StaticJam(const json& config, std::shared_ptr<Bus> bus, const std::filesystem::path& output_dir) 
    : JamObject(config, bus, output_dir) {

	eventBus = bus;
	eventBus->subscribe("NewStep", [this](std::shared_ptr<Event> eventData) {
		if (auto newStepData = std::dynamic_pointer_cast<NewStepEvent>(eventData)) {
			this->Update(newStepData);
		}
	});
}

void StaticJam::Update(std::shared_ptr<NewStepEvent> eventData){
    state.current_time = eventData->currentTime;
	auto newJamData = std::make_shared<JamEvent>(state);
	eventBus->publish("JamData", newJamData);
}

void StaticJam::PrintInfo() const {
	std::cout << "stat jam created" << std::endl;
}



