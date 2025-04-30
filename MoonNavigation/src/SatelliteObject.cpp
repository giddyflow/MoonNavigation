#include "SatelliteObject.h"

void SatelliteObject::InitPosition() {

}

SatelliteObject::SatelliteObject(const json& config, std::shared_ptr<Bus> bus) {
	mass = config["mass"];
	orbital_height = config["orbital_height"];
	state.Ptx = config["tx_power"];
	state.clock.shift = config["clock"]["shift"];
	state.clock.drift = config["clock"]["drift"];
	state.clock.instability = config["clock"]["instability"];
	state.id = config["id"];
	auto& std = config["std_dev"];
	std_dev.x = std["x"];
	std_dev.y = std["y"];
	std_dev.z = std["z"];

	//start = engine->getStartSecondsForEci();

	//eventBus = bus;
	//eventBus->subscribe("NewStep", [this](std::shared_ptr<Event> eventData) {
	//	//std::cout << "Received event: " << typeid(*eventData).name() << std::endl;
	//	auto newStepData = std::dynamic_pointer_cast<NewStepEvent>(eventData);
	//	if (newStepData) {
	//		this->Update(newStepData);
	//	}
	//	});
}

void SatelliteObject::setStartSeconds(double start_seconds) {
	start = start_seconds;
}


//void SatelliteObject::Update(std::shared_ptr<NewStepEvent> eventData) {
//	auto newSatData = std::make_shared<SatelliteEvent>(state);
//	eventBus->publish("SatData", newSatData);
//}