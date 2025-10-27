#include "AccessibilityAnalyzer.h"
#include <fstream>
#include <vector>
#include <cmath> // для round
#include <iomanip> // для setprecision

// Внешние функции
extern double deg2rad(double deg);
extern double rad2deg(double rad);
extern XYZ BLH2ECEF(BLH blh);
extern VisSat satVisabilityForViewPoint(const XYZ view_point_pos, const XYZ veiw_point_vel, const XYZ sat_pos, const XYZ sat_vel, const double mask);

AccessibilityAnalyzer::AccessibilityAnalyzer(const json& config, std::shared_ptr<Bus> bus, const std::filesystem::path& base_output_dir)
    : eventBus(bus), config(config), total_steps_processed(0) { // <-- ИЗМЕНЕНИЕ: инициализируем счетчик
    
    // <-- ИЗМЕНЕНИЕ: Метрика теперь жестко задана или проверяется
    metric = config.value("metric", "Availability"); 
    if (metric != "Availability") {
        std::cout << "Warning: This analyzer logic is designed for 'Availability' metric only." << std::endl;
    }
    outputFile = base_output_dir / config.value("output_file", "accessibility.json");
    
    mask_angle_rad = deg2rad(config["parameters"].value("mask_angle_deg", 7.5));
    min_visible_sats = config["parameters"].value("min_visible_sats", 4);
    
    initializeGrid();
    
    eventBus->subscribe("NewStep", [this](std::shared_ptr<Event> e) { this->handleNewStep(e); });
    eventBus->subscribe("MedSatData", [this](std::shared_ptr<Event> e) { this->handleSatelliteData(e); });
    eventBus->subscribe("Calc", [this](std::shared_ptr<Event> e) { this->handleCalc(e); });
    eventBus->subscribe("SaveAllResults", [this](std::shared_ptr<Event> e) { this->saveResults(e); });

    std::cout << "AccessibilityAnalyzer initialized for AVAILABILITY metric." << std::endl;
}

void AccessibilityAnalyzer::initializeGrid() {
    if (!config["grid"].value("is_global", false)) {
        std::cout << "AccessibilityAnalyzer: Non-global grid is not supported yet." << std::endl;
        return;
    }

    double lat_step = config["grid"].value("lat_step_deg", 10.0);
    double lon_step = config["grid"].value("lon_step_deg", 10.0);
    
    int grid_rows = static_cast<int>(180.0 / lat_step) + 1;
    int grid_cols = static_cast<int>(360.0 / lon_step) + 1;
    grid.reserve(grid_rows * grid_cols);

    for (double lat = -90.0; lat <= 90.0; lat += lat_step) {
        for (double lon = -180.0; lon <= 180.0; lon += lon_step) {
            GridPoint point;
            point.blh_coords = {deg2rad(lat), deg2rad(lon), 0.0}; 
            point.ecef_coords = BLH2ECEF(point.blh_coords);
            grid.push_back(point);
        }
    }
    // <-- ИЗМЕНЕНИЕ: Инициализируем вектор счетчиков нулями
    availability_counters.resize(grid.size(), 0);
}

void AccessibilityAnalyzer::handleNewStep(std::shared_ptr<Event> e) {
    // Эта функция теперь не делает ничего, так как очистка происходит в Calc
}

void AccessibilityAnalyzer::handleSatelliteData(std::shared_ptr<Event> e) {
    auto satEvent = std::dynamic_pointer_cast<SatelliteEvent>(e);
    if (!satEvent) return;
    current_step_sat_states.push_back(satEvent->satState);
}

// <-- ИЗМЕНЕНИЕ: Основная логика теперь здесь
void AccessibilityAnalyzer::handleCalc(std::shared_ptr<Event> e) {
    if (current_step_sat_states.empty()) {
        // Пропускаем шаг, если не было данных о спутниках
        current_step_sat_states.clear();
        return;
    }
    

    // Проходим по каждой точке сетки и обновляем ее счетчик
    for (size_t i = 0; i < grid.size(); ++i) {
        int visible_sats = countVisibleSatsForPoint(grid[i]);
        if (visible_sats >= min_visible_sats) {
            availability_counters[i]++;
        }
    }
    
    total_steps_processed++;
    current_step_sat_states.clear(); // Очищаем данные, готовясь к следующему шагу
}

// <-- ИЗМЕНЕНИЕ: Новая вспомогательная функция
int AccessibilityAnalyzer::countVisibleSatsForPoint(const GridPoint& point) {
    int count = 0;
    for (const auto& sat_state : current_step_sat_states) {
        VisSat vis_info = satVisabilityForViewPoint(point.ecef_coords, {0, 0, 0}, sat_state.eph.xyz, sat_state.eph.vxyz, rad2deg(mask_angle_rad));
        if (vis_info.visible) {
            count++;
        }
    }
    return count;
}

// <-- ИЗМЕНЕНИЕ: Полностью новая логика сохранения
void AccessibilityAnalyzer::saveResults(std::shared_ptr<Event> e) {
    json outputJson;

    // Заполняем метаданные
    outputJson["metadata"]["metric"] = metric;
    outputJson["metadata"]["total_steps_processed"] = total_steps_processed;
    outputJson["metadata"]["mask_angle_deg"] = rad2deg(mask_angle_rad);
    outputJson["metadata"]["min_visible_sats"] = min_visible_sats;

    json gridData = json::array();

    if (total_steps_processed == 0) {
        std::cout << "Warning: No steps were processed. Availability data will be empty." << std::endl;
    } else {
        for (size_t i = 0; i < grid.size(); ++i) {
            // Расчет процента доступности
            double availability_percent = (static_cast<double>(availability_counters[i]) / total_steps_processed) * 100.0;
            
            // Округление до одного знака после запятой
            availability_percent = std::round(availability_percent * 10.0) / 10.0;

            json pointData;
            pointData["lat"] = rad2deg(grid[i].blh_coords.lat);
            pointData["lon"] = rad2deg(grid[i].blh_coords.lon);
            pointData["availability_percent"] = availability_percent;
            gridData.push_back(pointData);
        }
    }
    
    outputJson["availability_grid"] = gridData;

    std::ofstream file(outputFile);
    if(file.is_open()) {
        file << outputJson.dump(4);
        file.close();
        std::cout << "Accessibility analysis results saved to " << outputFile.string() << std::endl;
    } else {
        std::cerr << "Error: Could not open file to save accessibility results: " << outputFile.string() << std::endl;
    }
}