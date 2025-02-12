#pragma once
#ifndef RECEIVEROBJECT_H
#define RECEIVEROBJECT_H

#include "Service.h"

struct RecState {
	XYZ ecef;
	BLH blh;
	XYZ velocity;
	int id;
};

class ReceiverObject{
protected:
	RecState state;
	const double suppress_coeff;
	const double mask;
	const double snr_treshold;
	const double power_spectral_density;
public:
	ReceiverObject(const std::string& type, const json& config) {
	
	}
	virtual ~ReceiverObject() = default;
	virtual void Update() = 0;
	virtual void CalcPosition() = 0;
};
#endif // !RECEIVEROBJECT_H