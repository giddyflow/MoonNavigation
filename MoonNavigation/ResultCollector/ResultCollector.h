#pragma once
#include "Service.h"
#include "Engine.h"          
#include "SatelliteObject.h" 
#include "ReceiverObject.h" 
#include "JamObject.h"      

class ResultCollector {
private:
    json m_results;
    std::filesystem::path m_output_path;

    void handleSatelliteEvent(const std::shared_ptr<SatelliteEvent>& event);
    void handleReceiverEvent(const std::shared_ptr<ReceiverEvent>& event);
    void handleJamEvent(const std::shared_ptr<JamEvent>& event);
    void saveResults();

public:
    ResultCollector(std::shared_ptr<Bus> bus, const std::filesystem::path& output_dir);
};