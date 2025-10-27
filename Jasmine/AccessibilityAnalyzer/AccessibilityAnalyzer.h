#pragma once

#include "EventBus.h"
#include "SatelliteObject.h"

#include <Eigen/Dense>
#include <filesystem>
#include <iostream>
#include <vector>

class AccessibilityAnalyzer {
public:
    AccessibilityAnalyzer(const json& config, std::shared_ptr<Bus> bus, const std::filesystem::path& base_output_dir);

private:
    struct GridPoint {
        BLH blh_coords;
        XYZ ecef_coords;
    };

    void handleNewStep(std::shared_ptr<Event> e);
    void handleSatelliteData(std::shared_ptr<Event> e);
    void handleCalc(std::shared_ptr<Event> e);
    void saveResults(std::shared_ptr<Event> e);

    void initializeGrid();
    // <-- ИЗМЕНЕНИЕ: Вспомогательная функция, вынесенная из старого calculateMetricForPoint
    int countVisibleSatsForPoint(const GridPoint& point); 

    std::shared_ptr<Bus> eventBus;
    json config;
    std::filesystem::path outputFile;

    std::string metric;
    double mask_angle_rad;
    int min_visible_sats;

    std::vector<GridPoint> grid;
    
    // <-- ИЗМЕНЕНИЕ: Новые поля для хранения состояния
    std::vector<int> availability_counters; // Счетчик "успешных" шагов для каждой точки
    int total_steps_processed;              // Общий счетчик шагов
    std::vector<SatState> current_step_sat_states;
};