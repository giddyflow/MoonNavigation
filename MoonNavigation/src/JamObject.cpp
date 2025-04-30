#include "JamObject.h"

JamObject::JamObject(const json& config, std::shared_ptr<Bus> bus) {
	tx_power = config["tx_power"];
	auto& start = config["start"];
	state.blh.lat = start["lat"];
	state.blh.lon = start["lon"];
	state.blh.h = start["h"];
}

//void JamObject::Update(std::shared_ptr<NewStepEvent> eventData) override {
//	auto newJamData = std::make_shared<JamEvent>(state);
//	eventBus->publish("JamData", newJamData);
//}