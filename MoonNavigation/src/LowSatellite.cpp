#include "LowSatellite.h"

LowSatellite::LowSatellite(const json& config)
    : SatelliteObject(config) {
}

void LowSatellite::Update() {
    // ��������, ����� �� ���� ������
    std::cout << "LowSatellite updating..." << std::endl;
}
