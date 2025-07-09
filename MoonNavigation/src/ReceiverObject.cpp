#include "ReceiverObject.h"

ReceiverObject::ReceiverObject(const json& config, std::shared_ptr<Bus> bus) {
	//state.current_time = 0;
	suppress_coeff = config["suppression_coeff"];
	mask = config["mask"];
	snr_treshold = config["snr_treshold"];
	power_spectral_density = config["PSD"];
	state.id = config["id"];

	//auto& speed = config["velocity"];
	//state.velocity.x = speed["x"];
	//state.velocity.y = speed["y"];
	//state.velocity.z = speed["z"];

	auto& start = config["start_coords"];
	state.blh.lat = start["lat"];
	state.blh.lon = start["lon"];
	state.blh.h = start["h"];
	auto& clock = config["clock"];
	state.clock.drift = clock["drift"];
	state.clock.shift = clock["shift"];
	clock_instability = clock["instability"];

	state.ecef = BLH2ECEF(state.blh);

	sats.clear();
	jams.clear();

	nap_key = "nap_" + std::to_string(state.id);
	nap_data_key = "data_" + nap_key;

	full_nap_json_data[nap_data_key] = json::object();
	full_nap_json_data[nap_data_key][nap_key] = json::object();
	full_nap_json_data[nap_data_key][nap_key]["dif"] = json::array();
	full_nap_json_data[nap_data_key][nap_key]["xyz_est_coords"] = json::array();
	full_nap_json_data[nap_data_key][nap_key]["xyz_model_coords"] = json::array();
	full_nap_json_data[nap_data_key][nap_key]["blh_est_coords"] = json::array();
	full_nap_json_data[nap_data_key][nap_key]["blh_model_coords"] = json::array();

	// std::filesystem::path logFilePath = "C:\\cpp_projects\\" + std::to_string(state.id) + ".txt";// "sat_" + std::to_string(state.id) + ".txt";
	// logFile.open(logFilePath, std::ios::out);

	// if (logFile.is_open()) {
	// 	createLogHeader();
	// }

	//eventBus->subscribe("NewStep", [this](std::shared_ptr<Event> eventData) {
	//	//std::cout << "Received event: " << typeid(*eventData).name() << std::endl;
	//	auto newStepData = std::dynamic_pointer_cast<NewStepEvent>(eventData);
	//	if (newStepData) {
	//		this->Update(newStepData);
	//	}
	//	});
}

double ReceiverObject::CalcJamNoise() {
	double noise_Prx = 0;

	for (auto& jam : jams) {
		bool visability = GroundVisability(state.blh, jam.blh);

		if (visability) {
			double dist = norm(jam.blh, state.blh);
			noise_Prx += db2pow(rx_power(jam.p_tx, 1602e6, dist));
		}
	}
	return noise_Prx;
}

std::vector<XYZ> ReceiverObject::getSatVelocity() {
	std::vector<XYZ> sat_velo;
	for (const auto& sat : sats) {
		sat_velo.push_back(sat.velocity);
	}
	return sat_velo;
}

void ReceiverObject::getVisibleSats(double jams_power = 0) {
	for (auto& sat : sats) {
		auto visdata = satVisabilityForViewPoint(state.ecef, state.velocity, sat.ecef, sat.velocity, mask);

		visdata.id = sat.id;
		if (visdata.visible) {
			visdata.Prx = rx_power(sat.Ptx, 1602e6, visdata.dist);
			visdata.q = CalcSNR(visdata.Prx, pow2db(jams_power));
			visdata.velECEF = sat.velocity;
			visdata.ephemeris = sat.eph;
			//visdata.q = calcSNR(visdata.Prx, -INFINITY);
		}
		visible_sats.push_back(visdata);

	}
}

double ReceiverObject::CalcSNR(double rxPower, double jamPower) {
	double q;
	double q1 = pow2db(db2pow(rxPower) / (db2pow(power_spectral_density))); // without jam
	double q2 = pow2db(db2pow(rxPower) / (db2pow(power_spectral_density) + db2pow(jamPower))); // with jam

	q = q2;
	//    if (q1 - q2 <= this->m_data.getKp()) q = q1;
	//    else q = q2 + this->m_data.getKp();
	return q;
}

void ReceiverObject::CalcPosition() {
	auto [dist, satpos, vsatpos] = makePseudoRangeMeasurements();
	auto res = mnkPseudoRangeCoordsTime(dist, satpos, state.ecef);
	auto [vr, _] = makePseudoVeloMeasurements();
	auto vrcorr = correctionVelo(vr, vsatpos, satpos, state.ecef);
	auto res_velo = mnkPseudoVeloCoordsTime(vrcorr, satpos, state.velocity, state.ecef);



	auto dop = CalcPseudoRangeDOP(satpos, state.ecef);
	state.dop = dop;
	state.est = res.flag;
	
	state.est_ecef.x = res.res.x;
	state.est_ecef.y = res.res.y;
	state.est_ecef.z = res.res.z;
	
	state.est_velocity.x = res_velo.res.x;
	state.est_velocity.y = res_velo.res.y;
	state.est_velocity.z = res_velo.res.z;
	
	state.est_clock = res.res.T / EarthConstants::c;
	state.est_drift = res_velo.res.T / EarthConstants::c;

	state.residual.x = state.ecef.x - state.est_ecef.x;
	state.residual.y = state.ecef.y - state.est_ecef.y;
	state.residual.z = state.ecef.z - state.est_ecef.z;
}


std::pair<std::vector<double>, std::vector<XYZ>> ReceiverObject::makeRangeMeasurements() {
	std::vector<double> dist;
	std::vector<XYZ> satpos;
	for (auto sat : visible_sats) {
		if (sat.visible) {
			dist.push_back(sat.dist + normrnd(0, 10.));
			satpos.push_back(sat.posECEF);
		}
	}
	return { dist, satpos };
}
std::vector<double> ReceiverObject::correctionVelo(std::vector<double> vr, std::vector<XYZ> vsatpos, std::vector<XYZ> satpos, XYZ res) {
	int N = vr.size();
	std::vector<double> vrcorr;
	Eigen::Vector3d delta;
	Eigen::Vector3d Vsat;
	double temp;

	for (int i = 0; i < N; i++) {
		Vsat(0) = vsatpos.at(i).x;
		Vsat(1) = vsatpos.at(i).y;
		Vsat(2) = vsatpos.at(i).z;
		delta << satpos.at(i).x - res.x, satpos.at(i).y - res.y, satpos.at(i).z - res.z;
		temp = (delta.transpose() * Vsat)(0) / delta.norm();
		vrcorr.push_back(vr[i] - temp);

		Vsat.setZero();
		delta.setZero();
	}


	return vrcorr;
}

std::pair<std::vector<double>, std::vector<XYZ>> ReceiverObject::makePseudoVeloMeasurements() {
	std::vector<double> vr;
	std::vector<XYZ> satpos;
	for (auto& sat : visible_sats) {
		if (sat.visible) {
			vr.push_back(sat.vr + state.clock.drift * EarthConstants::c);
			//vr.push_back(sat.vr);
			satpos.push_back(sat.posECEF);
		}
	}
	return { vr, satpos };
}



std::tuple<std::vector<double>, std::vector<XYZ>, std::vector<XYZ>> ReceiverObject::makePseudoRangeMeasurements() {
	std::vector<double> pseudorange;
	std::vector<XYZ> satpos;
	std::vector<XYZ> Vsat;
	for (auto& sat : visible_sats) {
		if (sat.visible) {
			double time_std = measure_time_std_by_snr(sat.q);
			double range_std = EarthConstants::c * time_std;
			pseudorange.push_back(sat.dist + state.clock.shift * EarthConstants::c + normrnd(0, range_std));
			satpos.push_back(sat.posECEF);
			Vsat.push_back(sat.ephemeris.vxyz);
		}
	}
	return { pseudorange, satpos, Vsat };
}



void ReceiverObject::addId() {
    full_nap_json_data[nap_data_key][nap_key]["id"] = state.id;
}

void ReceiverObject::addMetrics() {
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

    full_nap_json_data[nap_data_key][nap_key]["metrics"] = metrics_entry;
}

void ReceiverObject::addCoordsDifference() {
    json dif_entry;
    dif_entry["dif"]["dif_x"] = state.ecef.x - state.est_ecef.x;
    dif_entry["dif"]["dif_y"] = state.ecef.y - state.est_ecef.y;
    dif_entry["dif"]["dif_z"] = state.ecef.z - state.est_ecef.z;
    dif_entry["timestep"] = state.current_time;
    
    full_nap_json_data[nap_data_key][nap_key]["dif"].push_back(dif_entry);
}


void ReceiverObject::addModelCoords() {
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

    full_nap_json_data[nap_data_key][nap_key]["blh_model_coords"].push_back(entry_model_coords_blh);
    full_nap_json_data[nap_data_key][nap_key]["xyz_model_coords"].push_back(entry_model_coords_ecef);
}

void ReceiverObject::addEstimatedCoords() {
    json entry_est_coords_blh;
    BLH coords_ecef_to_blh = ECEF2BLH(state.est_ecef);
    entry_est_coords_blh["coords"]["lat"] = coords_ecef_to_blh.lat;
    entry_est_coords_blh["coords"]["lon"] = coords_ecef_to_blh.lon;
    entry_est_coords_blh["coords"]["h"] = coords_ecef_to_blh.h;
    entry_est_coords_blh["timestep"] = state.current_time;

    json entry_est_coords_ecef;
    entry_est_coords_ecef["coords"]["x"] = state.est_ecef.x;
    entry_est_coords_ecef["coords"]["y"] = state.est_ecef.y;
    entry_est_coords_ecef["coords"]["z"] = state.est_ecef.z;
    entry_est_coords_ecef["timestep"] = state.current_time;

    full_nap_json_data[nap_data_key][nap_key]["blh_est_coords"].push_back(entry_est_coords_blh);
    full_nap_json_data[nap_data_key][nap_key]["xyz_est_coords"].push_back(entry_est_coords_ecef);
}

// void ReceiverObject::createLogHeader() {
// 	if (logFile.is_open()) {
// 		logFile << "t" << " " <<
// 			"x " << " " <<
// 			"y " << " " <<
// 			"z " << " " <<
// 			"shift " << " " <<
// 			"drift " << " " <<
// 			"est_flag " << " " <<
// 			"x_est " << " " <<
// 			"y_est " << " " <<
// 			"z_est " << " " <<
// 			"clock_est " << " " <<
// 			"XDOP " << " " <<
// 			"YDOP " << " " <<
// 			"VDOP " << " " <<
// 			"TDOP " << " " <<
// 			"Vx_est " << " " <<
// 			"Vy_est " << " " <<
// 			"Vz_est " << " " <<
// 			"Vx " << " " <<
// 			"Vy " << " " <<
// 			"Vz " << " " <<
// 			"estDrift " << " " <<

// 			std::endl;
// 	}
// }


// void ReceiverObject::log() {
// 	if (logFile.is_open()) {
// 		logFile << std::fixed << std::setprecision(16);
// 		logFile << state.current_time << " " <<
// 			state.ecef.x << " " <<
// 			state.ecef.y << " " <<
// 			state.ecef.z << " " <<
// 			state.clock.shift << " " <<
// 			state.clock.drift << " " <<
// 			state.est << " " <<
// 			state.est_ecef.x << " " <<
// 			state.est_ecef.y << " " <<
// 			state.est_ecef.z << " " <<
// 			state.est_clock << " " <<
// 			state.dop.XDOP << " " <<
// 			state.dop.YDOP << " " <<
// 			state.dop.VDOP << " " <<
// 			state.dop.TDOP << " " <<
// 			state.est_velocity.x << " " <<
// 			state.est_velocity.y << " " <<
// 			state.est_velocity.z << " " <<
// 			state.velocity.x << " " <<
// 			state.velocity.y << " " <<
// 			state.velocity.z << " " <<
// 			state.est_drift << " ";
// 		for (auto& sat : visible_sats) {
// 			logFile << sat.id << " " <<
// 				sat.visible << " " <<
// 				sat.Prx << " " <<
// 				sat.q << " ";
// 		}

// 		logFile << std::endl;
// 	}
// }

//void ReceiverObject::Update(std::shared_ptr<NewStepEvent> eventData) override {
//	auto newRecData = std::make_shared<ReceiverEvent>(state);
//	eventBus->publish("RecData", newRecData);
//}