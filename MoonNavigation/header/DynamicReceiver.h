#pragma once
#ifndef DYNAMICRECEIVER_H
#define DYNAMICRECEIVER_H

#include "ReceiverObject.h"

class DynamicReceiver : public ReceiverObject {
private:
	double speed;
	double course;
	BLH finish_coords;
	void LoxodromeStep();
	void EasyStep();
	void Update() override;
	void CalcPosition() override;
public:
	DynamicReceiver(const json& config);
	void PrintInfo() const override {
		std::cout << "dyn rec created" << std::endl;
	}
};
#endif // !DYNAMICRECEIVER_H