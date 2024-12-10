#pragma once
#include "main.h"
#include "movement/chassis.hpp"
#include "pros/misc.hpp"
#include "core/subsystem.hpp"
#include <memory>

namespace movement {

// Driver control modes
enum class DriveMode {
    ARCADE,     // Single stick arcade
    SPLIT,      // Split arcade (drive/turn on separate sticks)
    TANK        // Traditional tank
};

// Driver control configuration
struct DriverConfig {
    DriveMode mode = DriveMode::ARCADE;
    double curve_factor = 1.5;      // Input curve for smoother control
    double deadzone = 0.05;         // Joystick deadzone
    double turn_scale = 0.8;        // Turn speed scaling
    int controller_id = 0;          // Primary = 0, Partner = 1
};

// Driver control class that works with our chassis
template<typename ChassisConfig>
class DriverControl : public core::ISubsystem {
private:
    std::string name_;
    Chassis<ChassisConfig>& chassis_;
    DriverConfig config_;
    pros::Controller controller_;
    bool enabled_ = false;

    // Input processing
    double applyDeadzone(double input) const {
        return std::abs(input) < config_.deadzone ? 0.0 : input;
    }

    double applyCurve(double input) const {
        return std::pow(std::abs(input), config_.curve_factor) * (input < 0 ? -1 : 1);
    }

    void processTankDrive() {
        if (!enabled_) return;
        
        double left = applyDeadzone(controller_.get_analog(ANALOG_LEFT_Y) / 127.0);
        double right = applyDeadzone(controller_.get_analog(ANALOG_RIGHT_Y) / 127.0);
        
        left = applyCurve(left);
        right = applyCurve(right);

        // Apply to left/right motor groups
        for (size_t i = 0; i < chassis_.getMotorCount(); i++) {
            bool is_left = i < chassis_.getMotorCount()/2;
            chassis_.setMotorVelocity(i, (is_left ? left : right) * 200.0);
        }
    }

    void processArcadeDrive(bool split) {
        if (!enabled_) return;

        double drive, turn;
        if (split) {
            drive = applyDeadzone(controller_.get_analog(ANALOG_LEFT_Y) / 127.0);
            turn = applyDeadzone(controller_.get_analog(ANALOG_RIGHT_X) / 127.0);
        } else {
            drive = applyDeadzone(controller_.get_analog(ANALOG_LEFT_Y) / 127.0);
            turn = applyDeadzone(controller_.get_analog(ANALOG_LEFT_X) / 127.0);
        }

        drive = applyCurve(drive);
        turn = applyCurve(turn) * config_.turn_scale;

        double left = drive + turn;
        double right = drive - turn;

        // Normalize if over ±1.0
        double max = std::max(std::abs(left), std::abs(right));
        if (max > 1.0) {
            left /= max;
            right /= max;
        }

        // Apply to motors
        for (size_t i = 0; i < chassis_.getMotorCount(); i++) {
            bool is_left = i < chassis_.getMotorCount()/2;
            chassis_.setMotorVelocity(i, (is_left ? left : right) * 200.0);
        }
    }

public:
    DriverControl(const std::string& name, Chassis<ChassisConfig>& chassis, 
                 const DriverConfig& config = DriverConfig())
        : name_(name)
        , chassis_(chassis)
        , config_(config)
        , controller_(config.controller_id == 0 ? 
              pros::Controller(pros::E_CONTROLLER_MASTER) 
            : pros::Controller(pros::E_CONTROLLER_PARTNER)) {}

    // ISubsystem interface implementation
    void initialize() override { enabled_ = true; }
    void update() override {
        if (!enabled_) return;
        
        switch (config_.mode) {
            case DriveMode::TANK:
                processTankDrive();
                break;
            case DriveMode::ARCADE:
                processArcadeDrive(false);
                break;
            case DriveMode::SPLIT:
                processArcadeDrive(true);
                break;
        }
    }
    
    void disable() override { 
        enabled_ = false;
        chassis_.stop();
    }
    
    bool isEnabled() const override { return enabled_; }
    const std::string& getName() const override { return name_; }

    // Configuration methods
    void setMode(DriveMode mode) { config_.mode = mode; }
    void setCurveFactor(double factor) { config_.curve_factor = factor; }
    void setDeadzone(double deadzone) { config_.deadzone = deadzone; }
    void setTurnScale(double scale) { config_.turn_scale = scale; }

    // Get current config
    const DriverConfig& getConfig() const { return config_; }
};

} // namespace movement