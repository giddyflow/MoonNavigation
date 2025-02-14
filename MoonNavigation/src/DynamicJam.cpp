#include "DynamicJam.h"

DynamicJam::DynamicJam(const json& config) : JamObject(config) {
	auto end = config["end"];
	finish_coords.lat = end["lat"];
	finish_coords.lon = end["lon"];
	finish_coords.h = end["h"];
}

void DynamicJam::Update() {

}



