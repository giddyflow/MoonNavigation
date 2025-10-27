#pragma once

#include "JamObject.h"

class DynamicJam : public JamObject {
private:
	BLH finish_coords;
	std::shared_ptr<Bus> eventBus;
	void Update(std::shared_ptr<NewStepEvent> new_step) override;
public:
	DynamicJam(const json& config, std::shared_ptr<Bus> bus, const std::filesystem::path& output_dir);
	void PrintInfo() const override {
		std::cout << "dyn jam created" << std::endl;
	}
};