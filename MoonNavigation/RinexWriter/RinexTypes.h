#pragma once
#include "Service.h" 

struct RinexObservation {
    int sat_id;
    char system;
    double c1;
    double l1;
    double d1;
    double s1;
};

struct RinexEpoch {
    std::vector<RinexObservation> obs;
};