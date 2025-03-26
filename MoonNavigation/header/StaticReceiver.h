#pragma once
#ifndef STATICRECEIVER_H
#define STATICRECEIVER_H

#include "ReceiverObject.h"

class StaticReceiver : public ReceiverObject {
private:
	std::shared_ptr<Bus> eventBus;
	void Update(std::shared_ptr<NewStepEvent> new_step) override {
		int a = 5;
	}
public:
	StaticReceiver(const json& config, std::shared_ptr<Bus> bus);
	void PrintInfo() const override {
		std::cout << "stat rec created" << std::endl;
	}
};
#endif // !DYNAMICRECEIVER_H