#pragma once
#ifndef SERVICE_H
#define SERVICE_H

#include <string>
#include <memory>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void ReadJson(const json& data);

struct XYZ {
	double x;
	double y;
	double z;
};

struct BLH {
	double lat;
	double lon;
	double h;
};
#endif // !SERVICE_H
