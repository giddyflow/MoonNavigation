#include "MediumSatellite.h"

MediumSatellite::MediumSatellite(const json& config, std::shared_ptr<Bus> bus, const std::filesystem::path& output_dir)
    : SatelliteObject(config, bus, output_dir) {
    initOrbitParams();
    //posUpdateSimpleOrbital(state.current_time);
    //ephUpdate();
    
    eventBus = bus;
    eventBus->subscribe("StartSeconds", [this](std::shared_ptr<Event> eventData) {
        if (auto startSecEvent = std::dynamic_pointer_cast<StartSecondsEvent>(eventData)) {
            this->setStartSeconds(startSecEvent->start_seconds);
            state.eci = ECEFtoECI(state.ecef, state.current_time, start_seconds);
            auto initialSatStateEvent = std::make_shared<SatelliteEvent>(state);
            eventBus->publish("MedSatData", initialSatStateEvent);
        }
    });

    eventBus->subscribe("NewStep", [this](std::shared_ptr<Event> eventData) {
        if (auto newStepData = std::dynamic_pointer_cast<NewStepEvent>(eventData)) {
            this->Update(newStepData);
        }
    });

}


void MediumSatellite::Update(std::shared_ptr<NewStepEvent> eventData) {
    posUpdateSimpleOrbital(eventData->currentTime);
    ephUpdate(); 
    state.current_time = eventData->currentTime;
    state.eci = ECEFtoECI(state.ecef, state.current_time, start_seconds);

    auto newMedSatData = std::make_shared<SatelliteEvent>(state);
    eventBus->publish("MedSatData", newMedSatData);
}

void MediumSatellite::initOrbitParams() {
    int number = state.id;
    double u = 45.;
    double v = 15.;
    switch (number)
    {
    case 1:
        theta0 = (number - 1) * u;
        OMEGA = 0;
        break;
    case 2:
        theta0 = -(number - 1) * u;
        OMEGA = 0;
        break;
    case 3:
        theta0 = -(number - 1) * u;
        OMEGA = 0;
        break;
    case 4:
        theta0 = -(number - 1) * u;
        OMEGA = 0;
        break;
    case 5:
        theta0 = -(number - 1) * u;
        OMEGA = 0;
        break;
    case 6:
        theta0 = -(number - 1) * u;
        OMEGA = 0;
        break;
    case 7:
        theta0 = -(number - 1) * u;
        OMEGA = 0;
        break;
    case 8:
        theta0 = -(number - 1) * u;
        OMEGA = 0;
        break;
    case 9:
        theta0 = -(number - 1 - 8) * u + 1 * v;
        OMEGA = 120;
        break;
    case 10:
        theta0 = -(number - 1 - 8) * u + 1 * v;
        OMEGA = 120;
        break;
    case 11:
        theta0 = -(number - 1 - 8) * u + 1 * v;
        OMEGA = 120;
        break;
    case 12:
        theta0 = -(number - 1 - 8) * u + 1 * v;
        OMEGA = 120;
        break;
    case 13:
        theta0 = -(number - 1 - 8) * u + 1 * v;
        OMEGA = 120;
        break;
    case 14:
        theta0 = -(number - 1 - 8) * u + 1 * v;
        OMEGA = 120;
        break;
    case 15:
        theta0 = -(number - 1 - 8) * u + 1 * v;
        OMEGA = 120;
        break;
    case 16:
        theta0 = -(number - 1 - 8) * u + 1 * v;
        OMEGA = 120;
        break;
    case 17:
        theta0 = -(number - 1 - 16) * u + 2 * v;
        OMEGA = 240;
        break;
    case 18:
        theta0 = -(number - 1 - 16) * u + 2 * v;
        OMEGA = 240;
        break;
    case 19:
        theta0 = -(number - 1 - 16) * u + 2 * v;
        OMEGA = 240;
        break;
    case 20:
        theta0 = -(number - 1 - 16) * u + 2 * v;
        OMEGA = 240;
        break;
    case 21:
        theta0 = -(number - 1 - 16) * u + 2 * v;
        OMEGA = 240;
        break;
    case 22:
        theta0 = -(number - 1 - 16) * u + 2 * v;
        OMEGA = 240;
        break;
    case 23:
        theta0 = -(number - 1 - 16) * u + 2 * v;
        OMEGA = 240;
        break;
    case 24:
        theta0 = -(number - 1 - 16) * u + 2 * v;
        OMEGA = 240;
        break;
    }

    theta0 = deg2rad(theta0);
    OMEGA = deg2rad(OMEGA);

}

void MediumSatellite::posUpdateSimpleOrbital(double t)
{
    inclination = 64.8015;
    double i_ = deg2rad(inclination);
    double r = orbital_height + EarthConstants::radius;
    double n = sqrt(EarthConstants::mu / pow(r, 3));

    double ci = cos(i_);
    double si = sin(i_);

    double Tsat = t * n;
    double Tear = -t * EarthConstants::omega_z; //   ECI  ECEF

    double V1 = -r * EarthConstants::omega_z;
    double V2 = r * n;

    double X = r * (cos(theta0 + Tsat) * cos(OMEGA + Tear) - sin(theta0 + Tsat) * sin(OMEGA + Tear) * ci); // (2.22) GLONASS 2020
    double Y = r * (cos(theta0 + Tsat) * sin(OMEGA + Tear) + sin(theta0 + Tsat) * cos(OMEGA + Tear) * ci); // (2.22) GLONASS 2020
    double Z = r * sin(theta0 + Tsat) * si; // (2.22) GLONASS 2020

    double VX = -V2 * (sin(theta0 + Tsat) * cos(OMEGA + Tear) + cos(theta0 + Tsat) * sin(OMEGA + Tear) * ci) - V1 * (cos(theta0 + Tsat) * sin(OMEGA + Tear) + sin(theta0 + Tsat) * cos(OMEGA + Tear) * ci);
    double VY = -V2 * (sin(theta0 + Tsat) * sin(OMEGA + Tear) - cos(theta0 + Tsat) * cos(OMEGA + Tear) * ci) + V1 * (cos(theta0 + Tsat) * cos(OMEGA + Tear) - sin(theta0 + Tsat) * sin(OMEGA + Tear) * ci);
    double VZ = V2 * cos(theta0 + Tsat) * si;

    state.ecef.x = X;
    state.ecef.y = Y;
    state.ecef.z = Z;
    state.velocity.x = VX;
    state.velocity.y = VY;
    state.velocity.z = VZ;
}

void MediumSatellite::ephUpdate() {
    double velosigma = 2;
    double shiftsigma = 1e-10;
    double driftsigma = 1e-12;
    state.eph.xyz.x = state.ecef.x + normrnd(0, std_dev.x);
    state.eph.xyz.y = state.ecef.y + normrnd(0, std_dev.y);
    state.eph.xyz.z = state.ecef.z + normrnd(0, std_dev.z);
    state.eph.vxyz.x = state.velocity.x + normrnd(0, velosigma);
    state.eph.vxyz.y = state.velocity.y + normrnd(0, velosigma);
    state.eph.vxyz.z = state.velocity.z + normrnd(0, velosigma);
    state.eph.clock.shift = state.clock.shift + normrnd(0, shiftsigma);
    state.eph.clock.drift = state.clock.drift + normrnd(0, driftsigma);
    
    state.residual.x = state.ecef.x - state.eph.xyz.x;
    state.residual.y = state.ecef.y - state.eph.xyz.y;
    state.residual.z = state.ecef.z - state.eph.xyz.z;
}

//void MediumSatellite::createLogHeader() {
//    if (logFile.is_open()) {
//        logFile << "t" << " " <<
//            "x" << " " <<
//            "y" << " " <<
//            "z" << " " <<
//            "vx" << " " <<
//            "vy" << " " <<
//            "vz" << " " <<
//            "shift" << " " <<
//            "drift" << " " <<
//            "eph_x" << " " <<
//            "eph_y" << " " <<
//            "eph_z" << " " <<
//            "eph_vx" << " " <<
//            "eph_vy" << " " <<
//            "eph_vz" << " " <<
//            "eph_shift" << " " <<
//            "eph_drift" << " " <<
//            std::endl;
//    }
//}
//
//
//void MediumSatellite::log() {
//    if (logFile.is_open()) {
//        logFile << std::fixed << std::setprecision(16);
//        logFile << state.current_time << " " <<
//            state.ecef.x << " " <<
//            state.ecef.y << " " <<
//            state.ecef.z << " " <<
//            state.velocity.z << " " <<
//            state.velocity.y << " " <<
//            state.velocity.z << " " <<
//            state.clock.shift << " " <<
//            state.clock.drift << " " <<
//            state.eph.xyz.x << " " <<
//            state.eph.xyz.y << " " <<
//            state.eph.xyz.z << " " <<
//            state.eph.vxyz.z << " " <<
//            state.eph.vxyz.y << " " <<
//            state.eph.vxyz.z << " " <<
//            state.eph.clock.shift << " " <<
//            state.eph.clock.drift << " " <<
//            std::endl;
//    }
//}
