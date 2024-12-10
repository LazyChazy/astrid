#pragma once
#include "main.h"
#include "robot_state.hpp"
#include "pros/llemu.hpp"
#include <string>
#include <functional>

namespace display {

class RobotDisplay {
private:
    RobotState& robot_;
    bool initialized_ = false;

    void updateStatus() {
        if (!initialized_) return;

        // Update clamp status
        std::string clamp_status = "Clamp: ";
        clamp_status += robot_.getClamp().isClamped() ? "ENGAGED" : "RELEASED";
        pros::lcd::set_text(1, clamp_status);

        // Update motor status
        std::string motor_text = "Motors: ";
        auto& chassis = robot_.getChassis();
        for (int i = 0; i < chassis.getMotorCount(); i++) {
            const auto& motor = chassis.getMotor(i);
            motor_text += "M" + std::to_string(i+1) + ":" + 
                         std::to_string(static_cast<int>(motor.get_actual_velocity())) + " ";
        }
        pros::lcd::set_text(2, motor_text);

        // Update mode status
        std::string mode_text = robot_.isDevMode() ? "DEV MODE" : "COMP MODE";
        pros::lcd::set_text(3, mode_text);

        // Check button presses manually
        if (pros::lcd::read_buttons() & LCD_BTN_LEFT) {
            robot_.getClamp().toggle();
            pros::delay(200); // Debounce
        }
        if (pros::lcd::read_buttons() & LCD_BTN_CENTER) {
            robot_.reset();
            pros::delay(200); // Debounce
        }
    }

public:
    RobotDisplay(RobotState& robot) : robot_(robot) {
        // Initialize LCD
        pros::lcd::initialize();
        initialized_ = true;

        // Display initial interface
        pros::lcd::set_text(0, "== Robot Control ==");
        pros::lcd::set_text(4, "L:Clamp C:Reset");
    }

    void update() {
        updateStatus();
    }

    void setStatusMessage(const std::string& msg) {
        if (initialized_) {
            pros::lcd::set_text(5, msg);
        }
    }
};

} // namespace display
