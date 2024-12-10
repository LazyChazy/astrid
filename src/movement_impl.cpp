#include "movement/chassis.hpp"
#include "movement/tank_chassis.hpp"
#include "movement/driver_control.hpp"
#include <memory>

// Robot configuration and control systems
namespace robot {

// Define the chassis type we want to use
using ChassisType = movement::TankChassis<movement::ChassisConfig<
    movement::DriveType::TANK,
    movement::OdomType::IMU_ENHANCED
>>;

// Global instances
std::unique_ptr<ChassisType> chassis;
std::unique_ptr<movement::DriverControl<movement::ChassisConfig<
    movement::DriveType::TANK,
    movement::OdomType::IMU_ENHANCED
>>> driver;

void initializeChassis() {
    // Create chassis instance
    chassis = std::make_unique<ChassisType>();
    
    // Add drive motors (port numbers are examples)
    chassis->addMotor(1, false);  // Left front
    chassis->addMotor(2, false);  // Left back
    chassis->addMotor(3, true);   // Right front
    chassis->addMotor(4, true);   // Right back
    
    // Initialize sensors
    chassis->initializeSensors(10); // IMU on port 10

    // Create driver control with custom configuration
    movement::DriverConfig driver_config;
    driver_config.mode = movement::DriveMode::SPLIT; // Split arcade drive
    driver_config.curve_factor = 1.8;  // More aggressive curve for finer control
    driver_config.turn_scale = 0.7;    // Reduce turning speed
    
    driver = std::make_unique<movement::DriverControl<movement::ChassisConfig<
        movement::DriveType::TANK,
        movement::OdomType::IMU_ENHANCED
    >>>("driver", *chassis, driver_config);  // Added name parameter
}

// Autonomous movement functions
void moveToGoal() {
    // Move to the center mobile goal
    chassis->moveTo(field::mobile_goals::CENTER.position);
    
    // Turn to face the high stake
    double angle = chassis->getPosition().angleTo(field::ladder::HIGH_STAKE.position);
    chassis->turnTo(angle);
}

// Driver control update function - call this in opcontrol loop
void updateDriverControl() {
    if (driver) {
        driver->update();
    }
}

// Example autonomous routine
void runAutonomous() {
    // Move to center goal
    chassis->moveTo(field::mobile_goals::CENTER.position);
    
    // Turn to face alliance goal
    chassis->turnTo(0); // Assuming 0 is facing alliance side
    
    // Move to another position
    chassis->moveTo(field::mobile_goals::BOTTOM_LEFT.position);
}

// Example operator control
void runOpControl() {
    while (true) {
        updateDriverControl();
        pros::delay(10); // Don't hog CPU
    }
}

} // namespace robot
