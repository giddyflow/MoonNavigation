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
	std::shared_ptr<Bus> eventBus;
	void Update(std::shared_ptr<NewStepEvent> new_step) override { int a = 5; }
public:
	DynamicReceiver(const json& config, std::shared_ptr<Bus> bus);
	void PrintInfo() const override {
		std::cout << "dyn rec created" << std::endl;
	}
};
#endif // !DYNAMICRECEIVER_H