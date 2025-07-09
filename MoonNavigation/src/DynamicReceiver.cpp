#include "DynamicReceiver.h"

DynamicReceiver::DynamicReceiver(const json& config, std::shared_ptr<Bus> bus) : ReceiverObject(config, bus){
	course = config["azimuth"];
    auto& speed = config["velocity"];
    dyn_state.velocity.x = speed["x"];
    dyn_state.velocity.y = speed["y"];
    dyn_state.velocity.z = speed["z"];
	auto& end = config["finish_coords"];
	dyn_state.finish_coords.lat = end["lat"];
    dyn_state.finish_coords.lon = end["lon"];
    dyn_state.finish_coords.h = end["h"];

    eventBus = bus;
    eventBus->subscribe("NewStep", [this](std::shared_ptr<Event> eventData) {
        auto newStepData = std::dynamic_pointer_cast<NewStepEvent>(eventData);
        if (newStepData) {
            this->Update(newStepData);
        }
        });

    eventBus->subscribe("MedSatData", [this](std::shared_ptr<Event> eventData) {
        auto newSatData = std::dynamic_pointer_cast<SatelliteEvent>(eventData);
        if (newSatData) {
            sats.push_back(newSatData->satState);
        }
        });
}


void DynamicReceiver::PrintInfo() const {
    std::cout << "dyn rec created" << std::endl;
}


void DynamicReceiver::Update(std::shared_ptr<NewStepEvent> new_step) {
    state.clock = markovModelOrder1(state.clock, new_step->currentTime - state.current_time, clock_instability);
    LoxodromeStep(new_step->currentTime - state.current_time);
    state.current_time = new_step->currentTime;
    void addId();
	void addMetrics();
	void addCoordsDifference();
	void addModelCoords();
	void addEstimatedCoords();
}

void DynamicReceiver::Calc() {
    visible_sats.clear();
    double noise_Prx = CalcJamNoise();
    getVisibleSats(noise_Prx);
    CalcPosition();
    sats.clear();
}

void DynamicReceiver::LoxodromeStep(double dt) {
    double theta, q, distance, step, ratio;
    double lat_step = state.blh.lat * M_PI / 180; // ������� -> �������
    double lon_step = state.blh.lon * M_PI / 180; // ������� -> �������
    double lat_finish = dyn_state.finish_coords.lat * M_PI / 180; // ������� -> �������
    double lon_finish = dyn_state.finish_coords.lon * M_PI / 180; // ������� -> �������
    double dLon = lon_finish - lon_step;
    double delta_psi = std::log(std::tan(M_PI / 4 + lat_finish / 2) / std::tan(M_PI / 4 + lat_step / 2));

    if (std::abs(delta_psi) > 1e-10) {
        theta = atan2(dLon, delta_psi);
        q = delta_psi / (lat_finish - lat_step);
    }
    else {
        theta = 0;
        q = std::cos(lat_step);
    }

    distance = EarthConstants::radius * sqrt(pow(lat_finish - lat_step, 2) + pow(q * dLon, 2));
    step = sqrt(pow(dyn_state.velocity.x, 2) + pow(dyn_state.velocity.y, 2) + pow(dyn_state.velocity.z, 2)) * dt;
    if (step >= distance) {
        return;
    }

    ratio = step / distance;
    dyn_state.blh.lat = (lat_step + ratio * (lat_finish - lat_step)) * 180 / M_PI;
    dyn_state.blh.lon = (lon_step + dLon * ratio) * 180 / M_PI;
    dyn_state.blh.h += ratio * (dyn_state.finish_coords.h - dyn_state.blh.h);


    dyn_state.blh.lon = std::fmod(dyn_state.blh.lon + 180.0, 360.0);
    if (dyn_state.blh.lon < 0) {
        dyn_state.blh.lon += 360.0;
    }
}


