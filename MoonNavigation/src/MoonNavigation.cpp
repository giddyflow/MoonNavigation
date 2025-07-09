#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include "ObjectFactory.h"
#include "Engine.h"

using json = nlohmann::json;

//static void ProcessEnvironment(const json& config, ConcreteObjectFactory& factory, std::shared_ptr<EventManager> manager) {
//    std::string place = config["place"];
//    const json& info = config["info"];
//    auto eng = factory.CreateEngine(place, info, manager);
//    if (eng) {
//        eng->PrintInfo();
//    }
//}


static void ProcessSatellites(const json& config, ConcreteObjectFactory& factory, std::vector<std::shared_ptr<Object>>& objs, std::shared_ptr<Bus>& bus) {
    if (config.contains("data") && config["data"].is_array()) {
        for (const auto& sat_data : config["data"]) {
            if (sat_data.contains("type")) {
                std::string type = sat_data["type"];
                auto satellite = factory.CreateSatellite(type, sat_data, bus);
                if (satellite) {
                    objs.push_back(satellite);
                    satellite->PrintInfo();
                }
            }
        }
    }
}

static void ProcessReceivers(const json& config, ConcreteObjectFactory& factory, std::vector<std::shared_ptr<Object>>& objs, std::shared_ptr<Bus>& bus) {
    if (config.contains("data") && config["data"].is_array()) {
        for (const auto& rec_data : config["data"]) {
            if (rec_data.contains("type")) {
                std::string type = rec_data["type"];
                auto receiver = factory.CreateReceiver(type, rec_data, bus);
                if (receiver) {
                    objs.push_back(receiver);
                    receiver->PrintInfo();
                }
            }
        }
    }
}

static void ProcessInterference(const json& config, ConcreteObjectFactory& factory, std::vector<std::shared_ptr<Object>>& objs, std::shared_ptr<Bus>& bus) {
    if (config.contains("data") && config["data"].is_array()) {
        for (const auto& jam_data : config["data"]) {
            if (jam_data.contains("type")) {
                std::string type = jam_data["type"];
                auto jam = factory.CreateJam(type, jam_data, bus);
                if (jam) {
                    objs.push_back(jam);
                    jam->PrintInfo();
                }
            }
        }
    }
}

static std::pair<json, std::vector<std::shared_ptr<Object>>> LoadAndCreateObjects(const std::filesystem::path folderPath, std::shared_ptr<Bus> bus) {

    std::filesystem::path configFile = folderPath / "in.json";
    std::ifstream file(configFile);
    json config;
    file >> config;
    std::cout << "Loaded JSON: " << config.dump(2) << std::endl;
    ConcreteObjectFactory factory;
    std::vector<std::shared_ptr<Object>> objs;
    if (config.contains("nap")) {
        ProcessReceivers(config["nap"], factory, objs, bus);
    }
    if (config.contains("nka")) {
        ProcessSatellites(config["nka"], factory, objs, bus);
    }
    if (config.contains("interference")) {
        ProcessInterference(config["interference"], factory, objs, bus);
    }
    //std::cout << "all config\n\n" << config.dump(4);
    return { config["controller"], objs};
}

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "Russian");
    //const std::string uri = "ws://194.58.96.141:9072"; // Используй порт WS (9071)
    //const std::string login = "user2"; // Твой логин
    //const std::string password = "DwfL~tfY#%1{";

    std::cout << "Received arguments:\n";
    for (int i = 0; i < argc; ++i) {
        std::cout << "argv[" << i << "] = " << argv[i] << "\n";
    }


    if (argc < 2) {
        std::cerr << "Укажите путь к папке, которая содержит in.json" << std::endl;
        return 1; 
    }

    std::filesystem::path folderPath = argv[1];
    std::filesystem::path configFilePath = folderPath / "in.json";
    if (!std::filesystem::exists(configFilePath)) {
        std::cerr << "ERROR: File not found - " << configFilePath << "\n";
        return 2;
    }

    setlocale(LC_ALL, "Russian");
    auto bus = std::make_shared<Bus>();
    auto [config, obj] = LoadAndCreateObjects(folderPath, bus);
    
    //auto [config, obj] = LoadAndCreateObjects("C:\\cpp_projects\\MoonNavigation\\MoonNavigation\\in.json", bus);
    
    std::cout << "Данные прочитаны\n";
    Engine engine(bus, config);
    engine.run();
}
