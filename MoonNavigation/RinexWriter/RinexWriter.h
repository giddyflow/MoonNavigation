#pragma once

#include "Service.h"
#include "RinexTypes.h"
#include "EventBus.h" 

class RinexWriter {
private:
    void createHeader(json config);
    void processEpoch(double epoch_time, const RinexEpoch& epoch);
    std::shared_ptr<Bus> eventBus;
    std::ofstream m_file;
public:
    RinexWriter(std::shared_ptr<Bus> bus, json config);
    ~RinexWriter();
};