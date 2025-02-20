#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include "ObjectFactory.h"

using json = nlohmann::json;

static void ProcessEnvironment(const json& config, ConcreteObjectFactory& factory) {
    std::string place = config["place"];
    const json& info = config["info"];
    auto env = factory.CreateEnvironment(place, info);
    if (env) {
        env->PrintInfo();
    }
}


static void ProcessSatellites(const json& config, ConcreteObjectFactory& factory) {
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

static void ProcessReceivers(const json& config, ConcreteObjectFactory& factory) {
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

static void ProcessInterference(const json& config, ConcreteObjectFactory& factory) {
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

static void LoadAndCreateObjects(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Ошибка: не удалось открыть файл " << filename << std::endl;
        return;
    }

    json config;
    try {
        file >> config;
    }
    catch (std::exception& e) {
        std::cerr << "Ошибка при чтении JSON: " << e.what() << std::endl;
        return;
    }

    ConcreteObjectFactory factory;

    if (config.contains("environment")) {
        ProcessEnvironment(config["environment"], factory);
    }
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
    LoadAndCreateObjects("C:\\cpp_projects\\MoonNavigation\\MoonNavigation\\config_file.json");
    std::cout << "data has been read\n";
}
