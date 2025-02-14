#pragma once
#ifndef STATICJAM_H
#define STATICJAM_H

#include "JamObject.h"

class StaticJam : public JamObject {
private:
	XYZ finish_ecef;
	BLH finish_blh;
public:
	StaticJam(const json& config);
	void Update() override;

	void PrintInfo() const override {
		std::cout << "stat jam created" << std::endl;
	}
};
#endif // !STATICJAM_H