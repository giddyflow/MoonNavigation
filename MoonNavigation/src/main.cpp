#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <vector>
#include <memory>

#include "ObjectFactory.h"
#include "Engine.h"
#include "RinexWriter.h" // Если он вам еще нужен
#include "ResultCollector.h" // <<< 1. ДОБАВЛЯЕМ НОВЫЙ INCLUDE

using json = nlohmann::json;

// Функции ProcessSatellites, ProcessReceivers, ProcessInterference остаются БЕЗ ИЗМЕНЕНИЙ.
// Они правильно создают объекты и не зависят от логики сохранения.
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

// Эта функция также остается БЕЗ ИЗМЕНЕНИЙ.
static SimulationSetup LoadAndCreateObjects(const std::filesystem::path& folderPath, std::shared_ptr<Bus> bus) {
    std::filesystem::path configFile = folderPath / "in.json";
    std::ifstream file(configFile);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open config file: " + configFile.string());
    }
    
    json config;
    file >> config;

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
        throw std::runtime_error("'controller' section not found in config file.");
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
    if (!std::filesystem::exists(folderPath / "in.json")) {
        std::cerr << "Ошибка: Конфигурационный файл не найден.\n";
        return 2;
    }

    try {
        auto bus = std::make_shared<Bus>();

        ResultCollector collector(bus, folderPath);

        SimulationSetup setup = LoadAndCreateObjects(folderPath, bus);

        RinexWriter rinex(bus, setup.engine_config);

        Engine engine(bus, setup.engine_config);
        engine.run();

    } catch (const std::exception& e) {
        std::cerr << "Критическая ошибка во время выполнения: " << e.what() << "\n";
        return 3;
    }

    return 0;
}