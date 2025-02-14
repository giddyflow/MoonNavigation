#pragma once
#ifndef DYNAMICJAM_H
#define DYNAMICJAM_H

#include "JamObject.h"

class DynamicJam : public JamObject {
private:
	BLH finish_coords;
public:
	DynamicJam(const json& config);
	void Update() override;
	void PrintInfo() const override {
		std::cout << "dyn jam created" << std::endl;
	}
};
#endif // !DYNAMICJAM_H