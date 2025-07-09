#include "JamObject.h"

JamObject::JamObject(const json& config, std::shared_ptr<Bus> bus) {
	state.id = config["id"];
	tx_power = config["power"];
	auto& start = config["coords"];
	state.blh.lat = start["lat"];
	state.blh.lon = start["lon"];
	state.blh.h = start["h"];
}

//void JamObject::Update(std::shared_ptr<NewStepEvent> eventData) override {
//	auto newJamData = std::make_shared<JamEvent>(state);
//	eventBus->publish("JamData", newJamData);
//}