#include "DynamicReceiver.h"

DynamicReceiver::DynamicReceiver(const json& config) : ReceiverObject(config) {
	course = config["course"];
	speed = config["speed"];
	auto end = config["end"];
	finish_coords.lat = end["lat"];
	finish_coords.lon = end["lon"];
	finish_coords.h = end["h"];
}

void DynamicReceiver::Update() {

}

void DynamicReceiver::CalcPosition() {

}
