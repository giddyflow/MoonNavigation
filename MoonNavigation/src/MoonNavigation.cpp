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
    for (const auto& sat_data : config) {
        if (sat_data.contains("type") && sat_data.contains("info")) {
            std::string type = sat_data["type"];
            const json& info = sat_data["info"];
            auto satellite = factory.CreateSatellite(type, info, bus);
            if (satellite) {
                objs.push_back(satellite);
                satellite->PrintInfo();
            }
        }
    }
}

static void ProcessReceivers(const json& config, ConcreteObjectFactory& factory, std::vector<std::shared_ptr<Object>>& objs, std::shared_ptr<Bus>& bus) {
    for (const auto& recv_data : config) {
        if (recv_data.contains("type") && recv_data.contains("info")) {
            std::string type = recv_data["type"];
            const json& info = recv_data["info"];
            auto receiver = factory.CreateReceiver(type, info, bus);
            if (receiver) {
                objs.push_back(receiver);
                receiver->PrintInfo();
            }
        }
    }
}

static void ProcessInterference(const json& config, ConcreteObjectFactory& factory, std::vector<std::shared_ptr<Object>>& objs, std::shared_ptr<Bus>& bus) {
    for (const auto& jam_data : config) {
        if (jam_data.contains("type") && jam_data.contains("info")) {
            std::string type = jam_data["type"];
            const json& info = jam_data["info"];
            auto jam = factory.CreateJam(type, info, bus);
            if (jam) {
                objs.push_back(jam);
                jam->PrintInfo();
            }
        }
    }
}

static std::pair<json, std::vector<std::shared_ptr<Object>>> LoadAndCreateObjects(const std::string& filename, std::shared_ptr<Bus> bus) {

    std::ifstream file(filename);
    json config;
    file >> config;
    ConcreteObjectFactory factory;
    std::vector<std::shared_ptr<Object>> objs;
    //if (config.contains("environment")) {
    //    ProcessEnvironment(config["environment"], factory, manager);
    //}
    if (config["objects"].contains("nap")) {
        ProcessReceivers(config["objects"]["nap"], factory, objs, bus);
    }
    if (config["objects"].contains("nka")) {
        ProcessSatellites(config["objects"]["nka"], factory, objs, bus);
    }
    if (config["objects"].contains("interference")) {
        ProcessInterference(config["objects"]["interference"], factory, objs, bus);
    }
    return { config["environment"], objs};
}

int main() {
    //const std::string uri = "ws://194.58.96.141:9072"; // Используй порт WS (9071)
    //const std::string login = "user2"; // Твой логин
    //const std::string password = "DwfL~tfY#%1{";

    setlocale(LC_ALL, "Russian");
    auto bus = std::make_shared<Bus>();
    auto [config, obj] = LoadAndCreateObjects("C:\\cpp_projects\\MoonNavigation\\MoonNavigation\\config_file.json", bus);

    std::cout << "data has been read\n";
    Engine engine(bus, config);
    engine.run();
}
