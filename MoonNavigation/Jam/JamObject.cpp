#include "JamObject.h"
#include <fstream>
#include <iostream>

JamObject::JamObject(const json& config, std::shared_ptr<Bus> bus, const std::filesystem::path& output_dir)
    : Object(config)
{
    state.id = this->id;
    state.p_tx = config["power"];
    auto& start = config["coords"];
    state.blh.lat = start["lat"];
    state.blh.lon = start["lon"];
    state.blh.h = start["h"];

    std::string jam_key = "jam_" + std::to_string(state.id);
    all_jammers_data[jam_key] = json::object();
    all_jammers_data[jam_key]["id"] = state.id;
    all_jammers_data[jam_key]["power"] = state.p_tx;
    all_jammers_data[jam_key]["coords"] = {
        {"lat", state.blh.lat}, 
        {"lon", state.blh.lon}, 
        {"h", state.blh.h}
    };
    all_jammers_data[jam_key]["trajectory"] = json::array();


    if (!save_handler_subscribed) {
        bus->subscribe("SaveAllResults", [output_dir](std::shared_ptr<Event> eventData){
            std::filesystem::path outputPath = output_dir / "interference.json";
            std::cout << "Saving all interference data to " << outputPath << "..." << std::endl;
            std::ofstream outFile(outputPath);
            if (outFile.is_open()) {
                outFile << all_jammers_data.dump(4);
            }
        });
        save_handler_subscribed = true;
    }
}