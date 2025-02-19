#pragma once
#ifndef OBJECTFACTORY_H
#define OBJECTFACTORY_H

#include "LowSatellite.h"
#include "GeoSatellite.h"
#include "MediumSatellite.h"
#include "StaticReceiver.h"
#include "DynamicReceiver.h"
#include "DynamicJam.h"
#include "StaticJam.h"
#include "ModelEnvironment.h"

class ObjectFactory {
public:
    virtual ~ObjectFactory() = default;
    virtual std::shared_ptr<SatelliteObject> CreateSatellite(const std::string& type, const json& config) = 0;
    virtual std::shared_ptr<ReceiverObject> CreateReceiver(const std::string& type, const json& config) = 0;
    virtual std::shared_ptr<JamObject> CreateJam(const std::string& type, const json& config) = 0;
    virtual std::shared_ptr<ModelEnvironment> CreateEnvironment(const std::string& place, const json& config) = 0;
};


class ConcreteObjectFactory : public ObjectFactory {
public:
    std::shared_ptr<SatelliteObject> CreateSatellite(const std::string& type, const json& config) override {
        if (type == "Low") return std::make_shared<LowSatellite>(config);
        if (type == "Geo") return std::make_shared<GeoSatellite>(config);
        if (type == "Medium") return std::make_shared<MediumSatellite>(config);
        return nullptr;
    }

    std::shared_ptr<ReceiverObject> CreateReceiver(const std::string& type, const json& config) override {
        if (type == "Static") return std::make_shared<StaticReceiver>(config);
        if (type == "Dynamic") return std::make_shared<DynamicReceiver>(config);
        return nullptr;
    }

    std::shared_ptr<JamObject> CreateJam(const std::string& type, const json& config) override {
        if (type == "Static") return std::make_shared<StaticJam>(config);
        if (type == "Dynamic") return std::make_shared<DynamicJam>(config);
        return nullptr;
    }

    std::shared_ptr<ModelEnvironment> CreateEnvironment(const std::string& place, const json& config) override {
        //надо будет сделать разные методы для луны и земли, сейчас подразумевается земля
        return std::make_shared<ModelEnvironment>(config);;
    }
};

#endif // !OBJECTFACTORY_H
