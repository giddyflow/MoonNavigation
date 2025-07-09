#include "SatelliteObject.h"

void SatelliteObject::InitPosition() {

}

SatelliteObject::SatelliteObject(const json& config, std::shared_ptr<Bus> bus) {
	//mass = config["mass"];
	orbital_height = config["orbital_height"];
	state.Ptx = config["sat_power"];
	state.clock.shift = config["clock"]["shift"];
	state.clock.drift = config["clock"]["drift"];
	state.clock.instability = config["clock"]["instability"];
	state.id = config["working_points"];
	auto& std = config["std_dev"];
	std_dev.x = std["x"];
	std_dev.y = std["y"];
	std_dev.z = std["z"];

    nka_key = "nka_" + std::to_string(state.id);
	nka_data_key = "data_" + nka_key;

	full_sat_json_data[nka_data_key] = json::object();
	full_sat_json_data[nka_data_key][nka_key] = json::object();
	full_sat_json_data[nka_data_key][nka_key]["dif"] = json::array();
	full_sat_json_data[nka_data_key][nka_key]["power"] = json::array();
	full_sat_json_data[nka_data_key][nka_key]["xyz_est_coords"] = json::array();
	full_sat_json_data[nka_data_key][nka_key]["xyz_model_coords"] = json::array();
	full_sat_json_data[nka_data_key][nka_key]["blh_est_coords"] = json::array();
	full_sat_json_data[nka_data_key][nka_key]["blh_model_coords"] = json::array();

	full_sat_json_data_eci[nka_data_key] = json::object();
	full_sat_json_data_eci[nka_data_key][nka_key] = json::object();
	full_sat_json_data_eci[nka_data_key][nka_key]["dif"] = json::array();
	full_sat_json_data_eci[nka_data_key][nka_key]["power"] = json::array();
	full_sat_json_data_eci[nka_data_key][nka_key]["xyz_est_coords"] = json::array();
	full_sat_json_data_eci[nka_data_key][nka_key]["xyz_model_coords"] = json::array();
	full_sat_json_data_eci[nka_data_key][nka_key]["blh_est_coords"] = json::array();
	full_sat_json_data_eci[nka_data_key][nka_key]["blh_model_coords"] = json::array();
	

	//start = engine->getStartSecondsForEci();

	//eventBus = bus;
	//eventBus->subscribe("NewStep", [this](std::shared_ptr<Event> eventData) {
	//	//std::cout << "Received event: " << typeid(*eventData).name() << std::endl;
	//	auto newStepData = std::dynamic_pointer_cast<NewStepEvent>(eventData);
	//	if (newStepData) {
	//		this->Update(newStepData);
	//	}
	//	});
}

void SatelliteObject::setStartSeconds(double start_seconds) {
    this->start_seconds = start_seconds;
}


void SatelliteObject::addId() {
    full_sat_json_data[nka_data_key][nka_key]["id"] = state.id;
	full_sat_json_data_eci[nka_data_key][nka_key]["id"] = state.id;
}

void SatelliteObject::addMetrics() {
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

    full_sat_json_data[nka_data_key][nka_key]["metrics"] = metrics_entry;
	full_sat_json_data_eci[nka_data_key][nka_key]["metrics"] = metrics_entry;
}

void SatelliteObject::addCoordsDifference() {
    json dif_entry;
    dif_entry["dif"]["dif_x"] = state.ecef.x - state.eph.xyz.x;
    dif_entry["dif"]["dif_y"] = state.ecef.y - state.eph.xyz.y;
    dif_entry["dif"]["dif_z"] = state.ecef.z - state.eph.xyz.z;
    dif_entry["timestep"] = state.current_time;
    
    full_sat_json_data[nka_data_key][nka_key]["dif"].push_back(dif_entry);
}

void SatelliteObject::addPower() {
    json power_entry;
    power_entry["power"] = state.Ptx;
    power_entry["timestep"] = state.current_time;
    
    full_sat_json_data[nka_data_key][nka_key]["power"].push_back(power_entry);
}

void SatelliteObject::addModelCoords() {
    json entry_model_coords_blh;
    BLH coords_ecef_to_blh = ECEF2BLH(state.ecef);
    entry_model_coords_blh["coords"]["lat"] = coords_ecef_to_blh.lat;
    entry_model_coords_blh["coords"]["lon"] = coords_ecef_to_blh.lon;
    entry_model_coords_blh["coords"]["h"] = coords_ecef_to_blh.h;

    json entry_model_coords_ecef;
    entry_model_coords_ecef["coords"]["x"] = state.ecef.x;
    entry_model_coords_ecef["coords"]["y"] = state.ecef.y;
    entry_model_coords_ecef["coords"]["z"] = state.ecef.z;

    entry_model_coords_blh["timestep"] = state.current_time;
    entry_model_coords_ecef["timestep"] = state.current_time;

    full_sat_json_data[nka_data_key][nka_key]["blh_model_coords"].push_back(entry_model_coords_blh);
    full_sat_json_data[nka_data_key][nka_key]["xyz_model_coords"].push_back(entry_model_coords_ecef);
}

void SatelliteObject::addEstimatedCoords() {
    json entry_est_coords_blh;
    BLH coords_ecef_to_blh = ECEF2BLH(state.eph.xyz);
    entry_est_coords_blh["coords"]["lat"] = coords_ecef_to_blh.lat;
    entry_est_coords_blh["coords"]["lon"] = coords_ecef_to_blh.lon;
    entry_est_coords_blh["coords"]["h"] = coords_ecef_to_blh.h;
    entry_est_coords_blh["timestep"] = state.current_time;

    json entry_est_coords_ecef;
    entry_est_coords_ecef["coords"]["x"] = state.eph.xyz.x;
    entry_est_coords_ecef["coords"]["y"] = state.eph.xyz.y;
    entry_est_coords_ecef["coords"]["z"] = state.eph.xyz.z;
    entry_est_coords_ecef["timestep"] = state.current_time;

    full_sat_json_data[nka_data_key][nka_key]["blh_est_coords"].push_back(entry_est_coords_blh);
    full_sat_json_data[nka_data_key][nka_key]["xyz_est_coords"].push_back(entry_est_coords_ecef);
}


//void SatelliteObject::Update(std::shared_ptr<NewStepEvent> eventData) {
//	auto newSatData = std::make_shared<SatelliteEvent>(state);
//	eventBus->publish("SatData", newSatData);
//}