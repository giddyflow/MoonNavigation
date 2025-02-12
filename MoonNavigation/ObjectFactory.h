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
            std::cout << "память под low выделена\n";
            return std::make_shared<LowSatellite>(type, config);
        }
        else if (type == "Geo") {
            std::cout << "память под geo выделена\n";
            return std::make_shared<GeoSatellite>(type, config);
        }
        else if (type == "Medium") {
            std::cout << "память под medium выделена\n";
            return std::make_shared<MediumSatellite>(type, config);
        }
        return nullptr;
    }

    std::shared_ptr<ReceiverObject> CreateReceiver(const std::string& type, const json& config) {
        if (type == "Static") {
            std::cout << "память под static выделена\n";
            return std::make_shared<StaticReceiver>(type, config);
        }
        else if (type == "Dynamic") {
            std::cout << "память под dynamic выделена\n";
            return std::make_shared<DynamicReceiver>(type, config);
        }
        return nullptr;
    }
};

#endif // !OBJECTFACTORY_H
