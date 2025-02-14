#pragma once
#ifndef SERVICE_H
#define SERVICE_H

#include <string>
#include <memory>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct Clock {
	double shift = 0;
	double drift = 0;
	double instability = 0;
};

struct XYZ {
	double x = 0;
	double y = 0;
	double z = 0;
};

struct BLH {
	double lat = 0;
	double lon = 0;
	double h = 0;
};


void ReadJson(const json& data);

#endif // !SERVICE_H
