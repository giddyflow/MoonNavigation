#include "DynamicJam.h"

DynamicJam::DynamicJam(const json& config, std::shared_ptr<Bus> bus) : JamObject(config, bus){
	auto& end = config["end"];
	finish_coords.lat = end["lat"];
	finish_coords.lon = end["lon"];
	finish_coords.h = end["h"];
}




