#include "main.h"
#include "robot_state.hpp"
#include "constants/fieldConstants.hpp"

void initialize() {
    // Configure the robot
    RobotConfig config;
    
    // Configure chassis
    config.chassis.left_motor_ports = {11, 20};     // Left front and back
    config.chassis.right_motor_ports = {1, 10};    // Right front and back
    config.chassis.imu_port = 9;                  // IMU port
    
    // Configure subsystems
    config.clamp.port = 'B';                      // Pneumatic clamp port
    
    // Configure driver controls
    config.driver.mode = movement::DriveMode::SPLIT;  // Split arcade drive
    
    // Initialize robot with configuration
    RobotState::getInstance(config);
}

void disabled() {
    core::SubsystemRegistry::getInstance().disableAll();
}

void competition_initialize() {}

void autonomous() {
    auto& robot = RobotState::getInstance();
    
    using ChassisConfig = movement::ChassisConfig<movement::DriveType::TANK, movement::OdomType::IMU_ENHANCED>;
    
    if (auto macro_system = robot.getSubsystemByType<movement::MacroSystem<ChassisConfig>>()) {
        // Create and register autonomous macro
        auto auton_macro = std::make_unique<movement::MovementMacro>([&robot]() {
            if (auto chassis = robot.getSubsystemByType<movement::Chassis<ChassisConfig>>()) {
                // Move forward
                for (size_t i = 0; i < chassis->getMotorCount(); i++) {
                    chassis->setMotorVelocity(i, 100); // 50% speed forward
                }
                pros::delay(1000);
                chassis->stop();
                
                // Move to specific point
                field::Point target_point(24, 0);
                chassis->moveTo(target_point);
            }
            
            if (auto clamp = robot.getSubsystem<subsystems::Clamp>("main_clamp")) {
                clamp->toggle();
            }
        });
        
        macro_system->registerMacro("auton_routine", std::move(auton_macro));
        macro_system->startMacro("auton_routine");
        
        // Run autonomous loop
        while (pros::competition::is_autonomous()) {
            robot.update();
            pros::delay(10);
        }
    }
}

void opcontrol() {
    auto& robot = RobotState::getInstance();
    
    // Main control loop
    while (true) {
        robot.update();
        pros::delay(10);
    }
}