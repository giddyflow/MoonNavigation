#pragma once
#ifndef MEDIUMSATELLITE_H
#define MEDIUMSATELLITE_H

#include "SatelliteObject.h"

class MediumSatellite : public SatelliteObject {
private:
	std::shared_ptr<Bus> eventBus;
public:
	MediumSatellite(const json& config, std::shared_ptr<Bus> bus);
	void initOrbitParams();
	void posUpdateSimpleOrbital(double t);
	void ephUpdate();
	void Update(std::shared_ptr<NewStepEvent> eventData) override;
	void PrintInfo() const override {
		std::cout << "medium Satellite created with id " << state.id << '\n';
	}
};
#endif // !MEDIUMSATELLITE_H