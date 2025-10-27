#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <vector>
#include <memory>

#include "ObjectFactory.h"
#include "Engine.h"
#include "RinexWriter.h" 
#include "ResultCollector.h" 
#include "../AccessibilityAnalyzer/AccessibilityAnalyzer.h"

using json = nlohmann::json;

static void ProcessSatellites(const json& config, ConcreteObjectFactory& factory, std::vector<std::shared_ptr<Object>>& objs, std::shared_ptr<Bus>& bus, const std::filesystem::path& output_dir) {
    if (config.contains("data") && config["data"].is_array()) {
        for (const auto& sat_data : config["data"]) {
            if (sat_data.contains("type")) {
                std::string type = sat_data["type"];
                auto satellite = factory.CreateSatellite(type, sat_data, bus, output_dir);
                if (satellite) {
                    objs.push_back(satellite);
                }
            }
        }
    }
}

static void ProcessReceivers(const json& config, ConcreteObjectFactory& factory, std::vector<std::shared_ptr<Object>>& objs, std::shared_ptr<Bus>& bus, const std::filesystem::path& output_dir) {
    if (config.contains("data") && config["data"].is_array()) {
        for (const auto& rec_data : config["data"]) {
            if (rec_data.contains("type")) {
                std::string type = rec_data["type"];
                auto receiver = factory.CreateReceiver(type, rec_data, bus, output_dir);
                if (receiver) {
                    objs.push_back(receiver);
                }
            }
        }
    }
}

static void ProcessInterference(const json& config, ConcreteObjectFactory& factory, std::vector<std::shared_ptr<Object>>& objs, std::shared_ptr<Bus>& bus, const std::filesystem::path& output_dir) {
    if (config.contains("data") && config["data"].is_array()) {
        for (const auto& jam_data : config["data"]) {
            if (jam_data.contains("type")) {
                std::string type = jam_data["type"];
                auto jam = factory.CreateJam(type, jam_data, bus, output_dir);
                if (jam) {
                    objs.push_back(jam);
                }
            }
        }
    }
}

struct SimulationSetup {
    json engine_config;
    std::vector<std::shared_ptr<Object>> objects;
};

static SimulationSetup LoadAndCreateObjects(const json& config, const std::filesystem::path& folderPath, std::shared_ptr<Bus> bus) {

    ConcreteObjectFactory factory;
    std::vector<std::shared_ptr<Object>> objs;

    if (config.contains("nap")) {
        ProcessReceivers(config["nap"], factory, objs, bus, folderPath);
    }
    if (config.contains("nka")) {
        ProcessSatellites(config["nka"], factory, objs, bus, folderPath);
    }
    if (config.contains("interference")) {
        ProcessInterference(config["interference"], factory, objs, bus, folderPath);
    }

    if (!config.contains("controller")) {
        throw std::runtime_error("В конфигурационном файле отсутствует поле controller");
    }
    
    return { config["controller"], objs };
}

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "Russian");

    if (argc < 2) {
        std::cerr << "Ошибка: Не указан путь к папке с конфигурационным файлом.\n";
        return 1;
    }

    std::filesystem::path folderPath = argv[1];
    std::filesystem::path configFilePath = folderPath / "in.json"; 
    if (!std::filesystem::exists(configFilePath)) {
        std::cerr << "Ошибка: Конфигурационный файл не найден.\n";
        return 2;
    }

        auto bus = std::make_shared<Bus>();

        std::ifstream file(configFilePath);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open config file: " + configFilePath.string());
        }
        json main_config;
        file >> main_config;

        ResultCollector collector(bus, folderPath);

        SimulationSetup setup = LoadAndCreateObjects(main_config, folderPath, bus);

        RinexWriter rinex(bus, setup.engine_config);

        std::shared_ptr<AccessibilityAnalyzer> analyzer;
        if (main_config.contains("accessibility_analysis") && main_config["accessibility_analysis"].value("enabled", false)) {
            analyzer = std::make_shared<AccessibilityAnalyzer>(main_config["accessibility_analysis"], bus, folderPath);
        }

        Engine engine(bus, setup.engine_config);
        engine.run();

    return 0;
}