#pragma once
#ifndef STATICRECEIVER_H
#define STATICRECEIVER_H

#include "ReceiverObject.h"

class StaticReceiver : public ReceiverObject {
private:
	void Update() override;
	void CalcPosition() override;
public:
	StaticReceiver(const json& config);
	void PrintInfo() const override {
		std::cout << "stat rec created" << std::endl;
	}
};
#endif // !DYNAMICRECEIVER_H