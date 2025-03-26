#define _USE_MATH_DEFINES 
#include "Service.h"

double rad2deg(double rad)
{
    return rad * 180 / M_PI;
}

double deg2rad(double deg)
{
    return deg * M_PI / 180;
}

double normrnd(double m, double sko) {
    //std::mt19937 generator;
    std::random_device rd;
    std::mt19937 generator(rd());
    std::normal_distribution<double> dist(m, sko);
    return dist(generator);
}

