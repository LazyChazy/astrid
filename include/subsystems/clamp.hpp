#pragma once
#include "main.h"
#include "pros/adi.hpp"
#include "core/subsystem.hpp"
#include <memory>

namespace subsystems {

// Clamp-specific configuration
struct ClampConfig : public core::SubsystemConfig {
    char port;
    bool default_state = false;
    
    ClampConfig(char port_num, bool dev = false) 
        : port(port_num) {
        dev_mode = dev;
    }
};

class Clamp : public core::Subsystem<ClampConfig> {
private:
    std::unique_ptr<pros::adi::DigitalOut> solenoid_;
    bool is_clamped_ = false;

    // Internal state change with event emission
    void setState(bool clamped) {
        if (is_clamped_ != clamped) {
            is_clamped_ = clamped;
            if (!config_.dev_mode && solenoid_) {
                solenoid_->set_value(is_clamped_);
            }
            // Emit state change event
            core::EventSystem::getInstance().emit("clamp_state_changed", is_clamped_);
        }
    }

public:
    Clamp(const std::string& name, const ClampConfig& config)
        : core::Subsystem<ClampConfig>(name, config) {}

    void initialize() override {
        if (!config_.dev_mode) {
            solenoid_ = std::make_unique<pros::adi::DigitalOut>(config_.port);
            setState(config_.default_state);
        }
        Subsystem::initialize();
    }

    void update() override {
        // Could add periodic checks or maintenance here
    }

    // Toggle clamp state
    void toggle() {
        setState(!is_clamped_);
    }

    // Directly set clamp state
    void setClamp(bool clamped) {
        setState(clamped);
    }

    // Get current state
    bool isClamped() const { return is_clamped_; }

    // Factory method for easy creation and registration
    static std::shared_ptr<Clamp> create(const std::string& name, char port, bool dev_mode = false) {
        ClampConfig config(port, dev_mode);
        auto clamp = std::make_shared<Clamp>(name, config);
        core::SubsystemRegistry::getInstance().registerSubsystem(clamp);
        return clamp;
    }
};

} // namespace subsystems