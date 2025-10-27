#pragma once

#include "SatelliteObject.h"

class MediumSatellite : public SatelliteObject {
private:
	std::shared_ptr<Bus> eventBus;
	std::ofstream logFile;
public:
	MediumSatellite(const json& config, std::shared_ptr<Bus> bus, const std::filesystem::path& output_dir);
	void initOrbitParams();
	void posUpdateSimpleOrbital(double t);
	void ephUpdate();
	//void log();
	//void createLogHeader();
	void Update(std::shared_ptr<NewStepEvent> eventData) override;
	void PrintInfo() const override {
		std::cout << "medium Satellite created with id " << state.id << '\n';
	}
};