#include "ModelEnvironment.h"

ModelEnvironment::ModelEnvironment(const json& config) {
	step = config["step"];
    stop_time = getStopTime(config);
}

std::tm ModelEnvironment::parseTime(const std::string& time_str) {
    std::tm tm = {};
    std::istringstream ss(time_str);
    ss >> std::get_time(&tm, "%Y %m %d %H %M %S");
    if (ss.fail()) {
        throw std::runtime_error("Error parsing time: " + time_str);
    }
    return tm;
}

double ModelEnvironment::getStopTime(const json& config) {
    std::string start_time_str = config["start_time"];
    std::string stop_time_str = config["stop_time"];
    std::tm start_tm = parseTime(start_time_str);
    std::tm stop_tm = parseTime(stop_time_str);
    std::time_t start = std::mktime(&start_tm);
    std::time_t stop = std::mktime(&stop_tm);
    return difftime(stop, start);
}