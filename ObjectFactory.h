#pragma once
#ifndef OBJECTFACTORY_H
#define OBJECTFACTORY_H

#include <memory>
#include <string>
#include "LowSatellite.h"
#include "GeoSatellite.h"
#include "MediumSatellite.h"

class ObjectFactory {
public:
	std::shared_ptr<SatelliteObject> CreateSatellite(const std::string& type, const json& config) {
		return nullptr;
	}

	std::shared_ptr<ReceiverObject> CreateReceiver(const std::string& type, const json& config) {
		return nullptr;
	}
};
#endif // !OBJECTFACTORY_H
