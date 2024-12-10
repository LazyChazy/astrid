#include "movement/control_system.hpp"
#include "movement/chassis.hpp"
#include "movement/driver_control.hpp"

namespace movement {

// Example helper function to setup a complete control system
template<typename ChassisConfig>
std::unique_ptr<EnhancedDriverControl<ChassisConfig>> setupControlSystem(
    const std::string& name,
    Chassis<ChassisConfig>& chassis,
    pros::Controller& controller
) {
    // Create macro system
    auto macro_system = std::make_unique<MacroSystem<ChassisConfig>>(
        name + "_macro",
        chassis
    );
    
    // Create input mapper
    auto input_mapper = std::make_unique<InputMapper<ChassisConfig>>(
        name + "_input",
        controller
    );

    // Register example macros
    
    // Quick turn 180 degrees macro
    macro_system->registerMacro("turn_180", std::make_unique<MovementMacro>([&chassis]() {
        chassis.turnTo(M_PI);
    }));

    // Drive square pattern macro
    macro_system->registerMacro("square_pattern", std::make_unique<MovementMacro>([&chassis]() {
        for (int i = 0; i < 4; i++) {
            chassis.moveTo(field::Point{24.0, 0.0});
            chassis.turnTo(M_PI/2);
        }
    }));

    // Setup input bindings
    
    // Button combo for 180-degree turn
    InputBinding turn_180_binding{
        .type = InputType::BUTTON_COMBO,
        .buttons = {DIGITAL_L1, DIGITAL_R1}
    };
    input_mapper->addBinding("turn_180", turn_180_binding, [&macro_system]() {
        macro_system->startMacro("turn_180");
    });

    // Button sequence for square pattern
    InputBinding square_pattern_binding{
        .type = InputType::SEQUENCE,
        .buttons = {DIGITAL_UP, DIGITAL_UP, DIGITAL_DOWN},
        .sequence_window = std::chrono::milliseconds(1000)
    };
    input_mapper->addBinding("square_pattern", square_pattern_binding, [&macro_system]() {
        macro_system->startMacro("square_pattern");
    });

    // Custom drive controls
    InputBinding forward_binding{
        .type = InputType::ANALOG_ABOVE,
        .analog = ANALOG_LEFT_Y,
        .threshold = 0.1
    };
    input_mapper->addBinding("drive_forward", forward_binding, [&chassis]() {
        for (size_t i = 0; i < chassis.getMotorCount(); i++) {
            chassis.setMotorVelocity(i, 200.0);
        }
    });

    // Create and return enhanced driver control
    return std::make_unique<EnhancedDriverControl<ChassisConfig>>(
        name,
        *macro_system,
        *input_mapper
    );
}

// Example function to run an autonomous routine
template<typename ChassisConfig>
void runAutonomousRoutine(MacroSystem<ChassisConfig>& macro_system) {
    macro_system.startMacro("turn_180");
    while (macro_system.isMacroActive()) {
        macro_system.update();
        pros::delay(10);
    }

    macro_system.startMacro("square_pattern");
    while (macro_system.isMacroActive()) {
        macro_system.update();
        pros::delay(10);
    }
}

} // namespace movement