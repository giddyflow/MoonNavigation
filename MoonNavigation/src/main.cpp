#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <filesystem> // Для работы с путями
#include <vector>
#include <memory>

#include "ObjectFactory.h"
#include "Engine.h"

using json = nlohmann::json;

static void ProcessSatellites(const json& config, ConcreteObjectFactory& factory, std::vector<std::shared_ptr<Object>>& objs, std::shared_ptr<Bus>& bus, const std::filesystem::path& output_dir) {
    if (config.contains("data") && config["data"].is_array()) {
        for (const auto& sat_data : config["data"]) {
            if (sat_data.contains("type")) {
                std::string type = sat_data["type"];
                auto satellite = factory.CreateSatellite(type, sat_data, bus, output_dir);
                if (satellite) {
                    objs.push_back(satellite);
                    // satellite->PrintInfo(); // Вывод информации можно оставить для отладки
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
                // Передаем output_dir в фабрику
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
                    // jam->PrintInfo();
                }
            }
        }
    }
}

struct SimulationSetup {
    json engine_config;
    std::vector<std::shared_ptr<Object>> objects;
};

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

    // Передаем folderPath как путь для вывода
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
        SimulationSetup setup = LoadAndCreateObjects(folderPath, bus);
        
        std::cout << "Объекты симуляции успешно созданы. Всего объектов: " << setup.objects.size() << "\n";
        
        Engine engine(bus, setup.engine_config);
        RinexWriter rinex(bus, setup.engine_config);
        engine.run();

    } catch (const std::exception& e) {
        std::cerr << "Критическая ошибка во время выполнения: " << e.what() << "\n";
        return 3;
    }

    return 0;
}