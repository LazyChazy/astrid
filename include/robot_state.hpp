#pragma once
#include "main.h"
#include "core/subsystem.hpp"
#include "movement/chassis.hpp"
#include "movement/tank_chassis.hpp"
#include "movement/control_system.hpp"
#include "movement/driver_control.hpp"
#include "subsystems/clamp.hpp"
#include <memory>

// Global configuration for the robot
struct RobotConfig {
    bool dev_mode = false;
    struct {
        std::vector<int> left_motor_ports;
        std::vector<int> right_motor_ports;
        int imu_port;
    } chassis;
    struct {
        char port;
    } clamp;
    struct {
        movement::DriveMode mode = movement::DriveMode::SPLIT;
    } driver;
};

class RobotState {
private:
    // Define chassis configuration type
    struct MainChassisConfig : movement::ChassisConfig<movement::DriveType::TANK, movement::OdomType::IMU_ENHANCED> {
        // Add any custom chassis configuration here if needed
    };
    
    RobotConfig config_;
    pros::Controller master_{pros::E_CONTROLLER_MASTER};
    
    // Singleton instance
    static std::unique_ptr<RobotState> instance_;

    // Private constructor for singleton
    explicit RobotState(const RobotConfig& config) : config_(config) {
        initializeSubsystems();
    }

    void initializeSubsystems() {
        auto& registry = core::SubsystemRegistry::getInstance();

        // Initialize chassis
        auto chassis = std::make_shared<movement::TankChassis<MainChassisConfig>>("main_chassis");
        
        // Configure motors
        for (int port : config_.chassis.left_motor_ports) {
            chassis->addMotor(port);
        }
        for (int port : config_.chassis.right_motor_ports) {
            chassis->addMotor(port, true);  // Right side reversed
        }

        if (!config_.dev_mode) {
            chassis->initializeSensors(config_.chassis.imu_port);
        }

        registry.registerSubsystem(chassis);

        // Initialize clamp subsystem
        auto clamp = subsystems::Clamp::create("main_clamp", config_.clamp.port, config_.dev_mode);

        // Initialize control systems
        auto driver = std::make_shared<movement::DriverControl<MainChassisConfig>>(
            "main_driver",
            *chassis,
            movement::DriverConfig{.mode = config_.driver.mode}
        );
        registry.registerSubsystem(driver);

        // Initialize input mapper and macro system
        auto input_mapper = std::make_shared<movement::InputMapper<MainChassisConfig>>(
            "main_input_mapper",
            master_
        );
        registry.registerSubsystem(input_mapper);

        auto macro_system = std::make_shared<movement::MacroSystem<MainChassisConfig>>(
            "main_macro",
            *chassis
        );
        registry.registerSubsystem(macro_system);

        // Initialize enhanced driver control
        auto enhanced_driver = std::make_shared<movement::EnhancedDriverControl<MainChassisConfig>>(
            "main_enhanced_driver",
            *macro_system,
            *input_mapper,
            movement::DriverConfig{.mode = config_.driver.mode}
        );
        registry.registerSubsystem(enhanced_driver);

        setupControls(input_mapper, clamp);
    }

    void setupControls(
        const std::shared_ptr<movement::InputMapper<MainChassisConfig>>& input_mapper,
        const std::shared_ptr<subsystems::Clamp>& clamp
    ) {
        if (input_mapper && clamp) {
            // Configure clamp control binding
            movement::InputBinding clamp_binding{
                .type = movement::InputType::BUTTON,
                .buttons = {pros::E_CONTROLLER_DIGITAL_R1}
            };
            input_mapper->addBinding("toggle_clamp", clamp_binding, 
                [clamp]() { clamp->toggle(); });
        }

        // Subscribe to clamp state changes for potential feedback
        core::EventSystem::getInstance().subscribe<bool>("clamp_state_changed",
            [this](const bool& is_clamped) {
                // Could add controller rumble or other feedback here
            });
    }

public:
    // Singleton access with configuration
    static RobotState& getInstance(const RobotConfig& config = RobotConfig()) {
        if (!instance_) {
            instance_ = std::unique_ptr<RobotState>(new RobotState(config));
        }
        return *instance_;
    }

    // Delete copy/move operations
    RobotState(const RobotState&) = delete;
    RobotState& operator=(const RobotState&) = delete;
    RobotState(RobotState&&) = delete;
    RobotState& operator=(RobotState&&) = delete;

    // Main update loop
    void update() {
        auto& registry = core::SubsystemRegistry::getInstance();
        registry.updateAll();
    }

    // Reset robot state
    void reset() {
        auto& registry = core::SubsystemRegistry::getInstance();
        
        if (!config_.dev_mode) {
            if (auto chassis = registry.getSubsystemByType<movement::Chassis<MainChassisConfig>>()) {
                chassis->initializeSensors(config_.chassis.imu_port);
            }
        }

        if (auto clamp = registry.getSubsystem<subsystems::Clamp>("main_clamp")) {
            clamp->setClamp(false);
        }
    }

    // Access subsystems through the registry
    template<typename T>
    std::shared_ptr<T> getSubsystem(const std::string& name) {
        return core::SubsystemRegistry::getInstance().getSubsystem<T>(name);
    }

    template<typename T>
    std::shared_ptr<T> getSubsystemByType() {
        return core::SubsystemRegistry::getInstance().getSubsystemByType<T>();
    }

    // Getter for clamp subsystem specifically
    subsystems::Clamp& getClamp() {
        if (auto clamp = getSubsystem<subsystems::Clamp>("main_clamp")) {
            return *clamp;
        }
        throw std::runtime_error("Clamp subsystem not initialized");
    }

    // Getter for chassis subsystem specifically
    movement::Chassis<MainChassisConfig>& getChassis() {
        if (auto chassis = getSubsystemByType<movement::Chassis<MainChassisConfig>>()) {
            return *chassis;
        }
        throw std::runtime_error("Chassis subsystem not initialized");
    }

    bool isDevMode() const { return config_.dev_mode; }
};

// Initialize static member
std::unique_ptr<RobotState> RobotState::instance_;