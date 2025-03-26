#pragma once
#ifndef LOWSATELLITE_H
#define LOWSATELLITE_H

#include "SatelliteObject.h"

class LowSatellite : public SatelliteObject {
private:
	std::shared_ptr<Bus> eventBus;
	void Update(std::shared_ptr<NewStepEvent> eventData) override {
		int a = 5;
	}
public:
	LowSatellite(const json& config, std::shared_ptr<Bus> bus);
	void PrintInfo() const override {
		std::cout << "low Satellite created with id " << state.id << '\n';
	}
};
#endif // !LOWSATELLITE_H
