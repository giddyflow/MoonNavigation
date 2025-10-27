#pragma once

#include "EventBus.h" 

class Object {
private:
    static inline int next_anonymous_id = -1;

public:
    int id;
    double current_time;

    explicit Object(const json& config) {
        if (config.contains("id") && config["id"].is_number()) {
            this->id = config["id"].get<int>();
        } else {
            this->id = next_anonymous_id--;
        }
        current_time = 0.0; 
    }

    virtual void Update(std::shared_ptr<NewStepEvent> eventData) = 0;
    virtual ~Object() = default;
};