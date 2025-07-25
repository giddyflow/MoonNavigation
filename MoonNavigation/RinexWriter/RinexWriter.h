#pragma once
#include "Service.h"
#include "Engine.h"

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

class RinexWriter {
private:
    void createHeader();
    void processEpoch();
    std::shared_ptr<Bus> eventBus;
    std::ofstream m_file;
public:
    RinexWriter(std::shared_ptr<Bus> bus, json date);
    ~RinexWriter();
};