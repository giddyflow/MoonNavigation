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

//#include "ModelEnvironment.h"

class ObjectFactory {
public:
    virtual ~ObjectFactory() = default;
    virtual std::shared_ptr<SatelliteObject> CreateSatellite(const std::string& type, const json& config, std::shared_ptr<Bus> bus) = 0;
    virtual std::shared_ptr<ReceiverObject> CreateReceiver(const std::string& type, const json& config, std::shared_ptr<Bus> bus) = 0;
    virtual std::shared_ptr<JamObject> CreateJam(const std::string& type, const json& config, std::shared_ptr<Bus> bus) = 0;
    //virtual std::shared_ptr<Engine> CreateEngine(const std::string& place, const json& config) = 0;
};


class ConcreteObjectFactory : public ObjectFactory {
public:
    std::shared_ptr<SatelliteObject> CreateSatellite(const std::string& type, const json& config, std::shared_ptr<Bus> bus) override {
        if (type == "Low") return std::make_shared<LowSatellite>(config, bus);
        if (type == "Geo") return std::make_shared<GeoSatellite>(config, bus);
        if (type == "Medium") return std::make_shared<MediumSatellite>(config, bus);
        return nullptr;
    }

    std::shared_ptr<ReceiverObject> CreateReceiver(const std::string& type, const json& config, std::shared_ptr<Bus> bus) override {
        if (type == "Static") return std::make_shared<StaticReceiver>(config, bus);
        if (type == "Dynamic") return std::make_shared<DynamicReceiver>(config, bus);
        return nullptr;
    }

    std::shared_ptr<JamObject> CreateJam(const std::string& type, const json& config, std::shared_ptr<Bus> bus) override {
        if (type == "Static") return std::make_shared<StaticJam>(config, bus);
        if (type == "Dynamic") return std::make_shared<DynamicJam>(config, bus);
        return nullptr;
    }

    //std::shared_ptr<Engine> CreateEngine(const std::string& place, const json& config) override {
    //    //надо будет сделать разные методы для луны и земли, сейчас подразумевается земля
    //    return std::make_shared<Engine>(config);;
    //}
};

#endif // !OBJECTFACTORY_H
