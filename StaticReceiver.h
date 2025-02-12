#pragma once
#ifndef STATICRECEIVER_H
#define STATICRECEIVER_H

#include "ReceiverObject.h"

class StaticReceiver : public ReceiverObject {
public:
	void Update() override;
};
#endif // !STATICRECEIVER_H