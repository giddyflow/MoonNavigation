#pragma once
#ifndef DYNAMICJAM_H
#define DYNAMICJAM_H

#include "JamObject.h"

class DynamicJam : public JamObject {
private:
	BLH finish_coords;
	std::shared_ptr<Bus> eventBus;
	void Update(std::shared_ptr<NewStepEvent> new_step) override {
		int a = 5;
	}
public:
	DynamicJam(const json& config, std::shared_ptr<Bus> bus);
	void PrintInfo() const override {
		std::cout << "dyn jam created" << std::endl;
	}
};
#endif // !DYNAMICJAM_H