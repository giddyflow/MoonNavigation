#pragma once
#ifndef STATICJAM_H
#define STATICJAM_H

#include "JamObject.h"

class StaticJam : public JamObject {
private:
	XYZ coords_ecef;
	BLH coords_blh;
	std::shared_ptr<Bus> eventBus;
	void Update(std::shared_ptr<NewStepEvent> new_step) override {
		int a = 10;
	}
public:
	StaticJam(const json& config, std::shared_ptr<Bus> bus);
	void PrintInfo() const override {
		std::cout << "stat jam created" << std::endl;
	}
};
#endif // !STATICJAM_H