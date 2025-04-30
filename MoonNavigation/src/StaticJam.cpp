#include "StaticJam.h"

StaticJam::StaticJam(const json& config, std::shared_ptr<Bus> bus) : JamObject(config, bus) {

	eventBus = bus;
	eventBus->subscribe("NewStep", [this](std::shared_ptr<Event> eventData) {
		//std::cout << "Received event: " << typeid(*eventData).name() << std::endl;
		auto newStepData = std::dynamic_pointer_cast<NewStepEvent>(eventData);
		if (newStepData) {
			this->Update(newStepData);
		}
		});
}

void StaticJam::Update(std::shared_ptr<NewStepEvent> eventData){
	auto newJamData = std::make_shared<JamEvent>(state);
	eventBus->publish("StatJamData", newJamData);
}

void StaticJam::PrintInfo() const {
	std::cout << "stat jam created" << std::endl;
}



