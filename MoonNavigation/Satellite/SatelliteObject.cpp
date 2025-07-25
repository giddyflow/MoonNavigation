#include "SatelliteObject.h"


SatelliteObject::SatelliteObject(const json& config, std::shared_ptr<Bus> bus, const std::filesystem::path& output_dir)
    : Object(config)
{
    orbital_height = config["orbital_height"];
    state.Ptx = config["sat_power"];
    state.clock.shift = config["clock"]["shift"];
    state.clock.drift = config["clock"]["drift"];
    state.clock.instability = config["clock"]["instability"];

    if (config.contains("working_points") && config["working_points"].is_number()) {
        this->id = config["working_points"].get<int>();
    }
    state.id = this->id;

    auto& std = config["std_dev"];
    std_dev.x = std["x"];
    std_dev.y = std["y"];
    std_dev.z = std["z"];

    // std::string nka_key = "nka_" + std::to_string(state.id);
    // all_sats_data_ecef[nka_key] = json::object();
    // all_sats_data_eci[nka_key] = json::object();

    // all_sats_data_ecef[nka_key]["dif"] = json::array();
    // all_sats_data_ecef[nka_key]["power"] = json::array();
    // all_sats_data_ecef[nka_key]["coords_est"] = json::array();
    // all_sats_data_ecef[nka_key]["blh_coords_model"] = json::array();
    // all_sats_data_ecef[nka_key]["blh_coords_est"] = json::array();
    // all_sats_data_ecef[nka_key]["xyz_model_coords"] = json::array();
    // all_sats_data_ecef[nka_key]["xyz_est_coords"] = json::array();

    // all_sats_data_eci[nka_key]["xyz_model_coords"] = json::array();

    // if (!save_handler_subscribed) {
    //     bus->subscribe("SaveAllResults", [output_dir](std::shared_ptr<Event> eventData) {
    //         std::filesystem::path outputPathEcef = output_dir / "nka.json";
    //         std::cout << "Saving all satellite ECEF data to " << outputPathEcef << "..." << std::endl;
    //         std::ofstream outFileEcef(outputPathEcef);
    //         if (outFileEcef.is_open()) outFileEcef << all_sats_data_ecef.dump(4);

    //         std::filesystem::path outputPathEci = output_dir / "nkaeci.json";
    //         std::cout << "Saving all satellite ECI data to " << outputPathEci << "..." << std::endl;
    //         std::ofstream outFileEci(outputPathEci);
    //         if (outFileEci.is_open()) outFileEci << all_sats_data_eci.dump(4);
    //     });
    //     save_handler_subscribed = true;
    // }
}


void SatelliteObject::setStartSeconds(double start_seconds) {
    this->start_seconds = start_seconds;
}


void SatelliteObject::addId(const std::string& key) {
    all_sats_data_ecef[key]["id"] = state.id;
    all_sats_data_eci[key]["id"] = state.id;
}

void SatelliteObject::addMetrics(const std::string& key) {
    json metrics_entry;
    metrics_entry["time_metrics"] = "секунды"; 
    metrics_entry["dif_metrics"] = "метры";
    metrics_entry["power_metrics"] = "дБВт";

    metrics_entry["coords_est_metrics"] = "метры"; 
    metrics_entry["coords_model_metrics"] = "метры";
    metrics_entry["blh_coords_est_metrics"] = "метры";

    metrics_entry["blh_coords_est_metrics_h"] = "метры"; 
    metrics_entry["blh_coords_model_metrics"] = "градусы";
    metrics_entry["blh_coords_model_metrics_h"] = "метры";

    all_sats_data_ecef[key]["metrics"] = metrics_entry;
    all_sats_data_eci[key]["metrics"] = metrics_entry;
}

void SatelliteObject::addCoordsDifference(const std::string& key) {
    json dif_entry;
    dif_entry["dif"]["dif_x"] = state.ecef.x - state.eph.xyz.x;
    dif_entry["dif"]["dif_y"] = state.ecef.y - state.eph.xyz.y;
    dif_entry["dif"]["dif_z"] = state.ecef.z - state.eph.xyz.z;
    dif_entry["timestep"] = state.current_time;
    
    all_sats_data_ecef[key]["dif"].push_back(dif_entry);
}

void SatelliteObject::addPower(const std::string& key) {
    json power_entry;
    power_entry["power"] = state.Ptx;
    power_entry["timestep"] = state.current_time;
    
    all_sats_data_ecef[key]["power"].push_back(power_entry);
}

void SatelliteObject::addModelCoords(const std::string& key) {
    json entry_model_coords_blh;
    BLH coords_ecef_to_blh = ECEF2BLH(state.ecef);
    entry_model_coords_blh["coords"]["lat"] = coords_ecef_to_blh.lat;
    entry_model_coords_blh["coords"]["lon"] = coords_ecef_to_blh.lon;
    entry_model_coords_blh["coords"]["h"] = coords_ecef_to_blh.h;
    entry_model_coords_blh["timestep"] = state.current_time;
    all_sats_data_ecef[key]["blh_model_coords"].push_back(entry_model_coords_blh);

    json entry_model_coords_ecef;
    entry_model_coords_ecef["coords"]["x"] = state.ecef.x;
    entry_model_coords_ecef["coords"]["y"] = state.ecef.y;
    entry_model_coords_ecef["coords"]["z"] = state.ecef.z;
    entry_model_coords_ecef["timestep"] = state.current_time;
    all_sats_data_ecef[key]["xyz_model_coords"].push_back(entry_model_coords_ecef);
}

void SatelliteObject::addEstimatedCoords(const std::string& key) {
    json entry_est_coords_blh;
    BLH coords_ecef_to_blh = ECEF2BLH(state.eph.xyz);
    entry_est_coords_blh["coords"]["lat"] = coords_ecef_to_blh.lat;
    entry_est_coords_blh["coords"]["lon"] = coords_ecef_to_blh.lon;
    entry_est_coords_blh["coords"]["h"] = coords_ecef_to_blh.h;
    entry_est_coords_blh["timestep"] = state.current_time;
    all_sats_data_ecef[key]["blh_est_coords"].push_back(entry_est_coords_blh);

    json entry_est_coords_ecef;
    entry_est_coords_ecef["coords"]["x"] = state.eph.xyz.x;
    entry_est_coords_ecef["coords"]["y"] = state.eph.xyz.y;
    entry_est_coords_ecef["coords"]["z"] = state.eph.xyz.z;
    entry_est_coords_ecef["timestep"] = state.current_time;
    all_sats_data_ecef[key]["xyz_est_coords"].push_back(entry_est_coords_ecef);
}

void SatelliteObject::addEciModelCoords(const std::string& key) {
    json entry_model_coords_eci;
    entry_model_coords_eci["coords"]["x"] = state.eci.x;
    entry_model_coords_eci["coords"]["y"] = state.eci.y;
    entry_model_coords_eci["coords"]["z"] = state.eci.z;
    entry_model_coords_eci["timestep"] = state.current_time;
    all_sats_data_eci[key]["xyz_model_coords"].push_back(entry_model_coords_eci);
}


//void SatelliteObject::Update(std::shared_ptr<NewStepEvent> eventData) {
//	auto newSatData = std::make_shared<SatelliteEvent>(state);
//	eventBus->publish("SatData", newSatData);
//}