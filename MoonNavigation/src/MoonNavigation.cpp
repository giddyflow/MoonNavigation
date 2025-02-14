#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include "ObjectFactory.h"

using json = nlohmann::json;

void ProcessSatellites(const json& config, ConcreteObjectFactory& factory) {
    for (const auto& sat_data : config) {
        if (sat_data.contains("type") && sat_data.contains("info")) {
            std::string type = sat_data["type"];
            const json& info = sat_data["info"];
            auto satellite = factory.CreateSatellite(type, info);
            if (satellite) {
                satellite->PrintInfo();
            }
        }
    }
}

void ProcessReceivers(const json& config, ConcreteObjectFactory& factory) {
    for (const auto& recv_data : config) {
        if (recv_data.contains("type") && recv_data.contains("info")) {
            std::string type = recv_data["type"];
            const json& info = recv_data["info"];
            auto receiver = factory.CreateReceiver(type, info);
            if (receiver) {
                receiver->PrintInfo();
            }
        }
    }
}

void ProcessInterference(const json& config, ConcreteObjectFactory& factory) {
    for (const auto& jam_data : config) {
        if (jam_data.contains("type") && jam_data.contains("info")) {
            std::string type = jam_data["type"];
            const json& info = jam_data["info"];
            auto jam = factory.CreateJam(type, info);
            if (jam) {
                jam->PrintInfo();
            }
        }
    }
}

void LoadAndCreateObjects(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Ошибка: не удалось открыть файл " << filename << std::endl;
        return;
    }

    json config;
    try {
        file >> config;
        //std::cout << config.dump(4) << '\n';
    }
    catch (std::exception& e) {
        std::cerr << "Ошибка при чтении JSON: " << e.what() << std::endl;
        return;
    }

    ConcreteObjectFactory factory;

    // Вызов вспомогательных функций для обработки объектов
    if (config["objects"].contains("nap")) {
        ProcessReceivers(config["objects"]["nap"], factory);
    }
    if (config["objects"].contains("nka")) {
        ProcessSatellites(config["objects"]["nka"], factory);
    }
    if (config["objects"].contains("interference")) {
        ProcessInterference(config["objects"]["interference"], factory);
    }
}



int main() {
    setlocale(LC_ALL, "Russian");
    LoadAndCreateObjects("C:\\Users\\Grigoriy.LAPTOP-U1U029UA\\source\\repos\\MoonNavigation\\MoonNavigation\\config_file.json");

}
