#include "ModelEnvironment.h"

ModelEnvironment::ModelEnvironment(const json& config) {
	step = config["step"];
}