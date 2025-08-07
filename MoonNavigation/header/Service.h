#pragma once
#define _USE_MATH_DEFINES 

#include <fstream>
#include <iostream> 
#include <sstream>

#include <algorithm>
#include <iomanip>
#include <functional>

#include <string>
#include <unordered_map>
#include <map>
#include <vector>

#include <random>
#include <memory>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <utility>

#include <nlohmann/json.hpp>
#include <Eigen/Dense>

using json = nlohmann::json;

namespace EarthConstants {
	constexpr double a = 6378136;
	constexpr double radius = 6356863;
	constexpr double mu = 3.986004418e14;
	constexpr double omega_z = 7.2921151467e-5;
	constexpr double c = 299792458;
	constexpr double f = 1 / 298.25784;
	constexpr double e2 = f * (2 - f);
	constexpr double sec_in_rad = M_PI / 43200;
}

struct Clock {
	double shift;
	double drift;
	double instability;
};

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

struct Ephemeris {
	XYZ xyz;
	XYZ vxyz;
	Clock clock;
};

struct DOP {
	double XDOP;
	double YDOP;
	double VDOP;
	double TDOP;
	double HDOP;
	double PDOP;
};

struct VisSat {
	int id;
	XYZ posECEF;
	XYZ velECEF;
	XYZ posENU;
	double az, el;
	double dist, vr;
	double q, Prx;
	bool visible;
	Ephemeris ephemeris;
};

struct ResultRangeMNK {
	bool flag;
	int iter = 0;
	double nev = 0;
	XYZ res;
};

struct XYZT {
	double x;
	double y;
	double z;
	double T;
};

struct ResultPseudoRangeMNK {
	bool flag;
	int iter = 0;
	double nev = 0;
	XYZT res;
};

struct ResultPseudoVeloMNK {
	bool flag;
	int iter = 0;
	double nev = 0;
	XYZT res;
};


double deg2rad(double deg);
double rad2deg(double rad);
double norm(BLH point1, BLH point2);
double normrnd(double m, double sko);
double pow2db(double x);
double db2pow(double x);
double rx_power(double txPower, double freq, double dist);
double measure_time_std_by_snr(double q);

bool GroundVisability(const BLH point1, const BLH point2);

XYZ BLH2ECEF(BLH blh);
XYZ BLH2ENU(BLH blh_ref, BLH blh);
XYZ ECEF2ENU(XYZ xyz_ref, XYZ xyz_post);
XYZ ENU2ECEF(BLH blh_ref, XYZ enu);
XYZ ECEFtoECI(XYZ xyz_ecef, const double S0, const double ti);
BLH ECEF2BLH(XYZ ecef);

Clock markovModelOrder1(Clock clock, double dt, double instability);


DOP CalcPseudoRangeDOP(std::vector<XYZ> posts, XYZ X0);

ResultPseudoVeloMNK mnkVeloCoordsTime(std::vector<double> vr, std::vector<XYZ> posts, XYZ V0, XYZ X0);
ResultPseudoVeloMNK mnkPseudoVeloCoordsTime(std::vector<double> vr, std::vector<XYZ> posts, XYZ V0, XYZ X0);
ResultPseudoRangeMNK mnkPseudoRangeCoordsTime(std::vector<double> d, std::vector<XYZ> posts, XYZ X0);
ResultRangeMNK mnkRangeCoords(std::vector<double> d, std::vector<XYZ> posts, XYZ X0);

VisSat satVisabilityForViewPoint(const XYZ view_point_pos,
	const XYZ veiw_point_vel, const XYZ sat_pos, const XYZ sat_vel, const double mask);

std::tm parseTime(const std::string& time_str);
std::pair<double, double> getStopTime(const json& config);
double secondsFromStartOfDay(const std::tm& time);