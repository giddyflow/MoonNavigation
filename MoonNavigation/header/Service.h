#pragma once
#ifndef SERVICE_H
#define SERVICE_H

#include <string>
#include <random>
#include <memory>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream> 
#include <unordered_map>
#include <vector>
#include <functional>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace EarthConstants {
	constexpr double radius = 6356863;
	constexpr double mu = 3.986004418e14;
	constexpr double omega_z = 7.2921151467e-5;
	constexpr double c = 299792458;
}

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

struct Ephemeris {
	XYZ xyz;
	XYZ vxyz;
	Clock clock;
};


double deg2rad(double deg);
double rad2deg(double rad);
double normrnd(double m, double sko);


#endif // !SERVICE_H
