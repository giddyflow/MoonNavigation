#pragma once
#ifndef DYNAMICRECEIVER_H
#define DYNAMICRECEIVER_H

#include "ReceiverObject.h"

struct DynRecState : RecState {
	BLH finish_coords;
};

class DynamicReceiver : public ReceiverObject {
private:
	DynRecState dyn_state;
	double course;
	void LoxodromeStep(double dt);
	//void EasyStep();
	std::shared_ptr<Bus> eventBus;
	void Update(std::shared_ptr<NewStepEvent> new_step) override;
public:
	DynamicReceiver(const json& config, std::shared_ptr<Bus> bus, const std::filesystem::path& output_dir);
	void Calc() override;
	void PrintInfo() const override;
};
#endif // !DYNAMICRECEIVER_H