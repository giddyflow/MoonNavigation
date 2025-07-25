#pragma once
#ifndef GEOSATELLITE_H
#define GEOSATELLITE_H

#include "SatelliteObject.h"

class GeoSatellite : public SatelliteObject {
private:
	std::shared_ptr<Bus> eventBus;
	void Update(std::shared_ptr<NewStepEvent> eventData) override {
	}
public:
	GeoSatellite(const json& config, std::shared_ptr<Bus> bus, const std::filesystem::path& output_dir);
	void PrintInfo() const override {
		std::cout << "geo Satellite created with id " << state.id << '\n';
	}
};
#endif // !GEOSATELLITE_H