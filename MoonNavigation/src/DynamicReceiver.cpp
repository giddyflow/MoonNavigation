#include "DynamicReceiver.h"

DynamicReceiver::DynamicReceiver(const json& config, std::shared_ptr<Bus> bus) : ReceiverObject(config, bus){
	course = config["course"];
	speed = config["speed"];
	auto& end = config["end"];
	finish_coords.lat = end["lat"];
	finish_coords.lon = end["lon"];
	finish_coords.h = end["h"];
	eventBus = bus;
}


