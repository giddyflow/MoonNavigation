#pragma once
#ifndef OBJECTFACTORY_H
#define OBJECTFACTORY_H

#include "LowSatellite.h"
#include "GeoSatellite.h"
#include "MediumSatellite.h"
#include "StaticReceiver.h"
#include "DynamicReceiver.h"

class ObjectFactory {
public:
    std::shared_ptr<SatelliteObject> CreateSatellite(const std::string& type, const json& config) {
        if (type == "Low") {
            std::cout << "������ ��� low ��������\n";
            return std::make_shared<LowSatellite>(type, config);
        }
        else if (type == "Geo") {
            std::cout << "������ ��� geo ��������\n";
            return std::make_shared<GeoSatellite>(type, config);
        }
        else if (type == "Medium") {
            std::cout << "������ ��� medium ��������\n";
            return std::make_shared<MediumSatellite>(type, config);
        }
        return nullptr;
    }

    std::shared_ptr<ReceiverObject> CreateReceiver(const std::string& type, const json& config) {
        if (type == "Static") {
            std::cout << "������ ��� static ��������\n";
            return std::make_shared<StaticReceiver>(type, config);
        }
        else if (type == "Dynamic") {
            std::cout << "������ ��� dynamic ��������\n";
            return std::make_shared<DynamicReceiver>(type, config);
        }
        return nullptr;
    }
};

#endif // !OBJECTFACTORY_H
