#pragma once
#ifndef DYNAMICRECEIVER_H
#define DYNAMICRECEIVER_H

#include "ReceiverObject.h"

class DynamicReceiver : public ReceiverObject {
private:
	double speed;
	double course;
	BLH finish_coords;
public:
	void Update() override;
};
#endif // !DYNAMICRECEIVER_H