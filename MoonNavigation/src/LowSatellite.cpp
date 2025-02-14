#include "LowSatellite.h"

LowSatellite::LowSatellite(const json& config)
    : SatelliteObject(config) {
}

void LowSatellite::Update() {
    // Заглушка, чтобы не было ошибки
    std::cout << "LowSatellite updating..." << std::endl;
}
