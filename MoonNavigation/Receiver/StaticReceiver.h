#pragma once

#include "ReceiverObject.h"

class StaticReceiver : public ReceiverObject {
private:
	std::shared_ptr<Bus> eventBus;
public:
	StaticReceiver(const json& config, std::shared_ptr<Bus> bus, const std::filesystem::path& output_dir);
	void Update(std::shared_ptr<NewStepEvent> new_step) override;
	void Calc() override;
	void PrintInfo() const override {
		std::cout << "stat rec created" << std::endl;
	}
};