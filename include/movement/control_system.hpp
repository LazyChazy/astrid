#pragma once
#include "main.h"
#include "movement/chassis.hpp"
#include "movement/driver_control.hpp"
#include "pros/misc.hpp"
#include "core/subsystem.hpp"
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <chrono>
#include <algorithm>

namespace movement {

// Forward declarations
template<typename ChassisConfig>
class MacroSystem;

template<typename ChassisConfig>
class InputMapper;

// Base macro interface
class Macro {
public:
    virtual ~Macro() = default;
    virtual void execute() = 0;
    virtual bool isComplete() const = 0;
    virtual void reset() = 0;
};

// Input binding types
enum class InputType {
    BUTTON,
    BUTTON_COMBO,
    ANALOG_ABOVE,
    ANALOG_BELOW,
    SEQUENCE
};

// Input binding structure
struct InputBinding {
    InputType type;
    std::vector<pros::controller_digital_e_t> buttons;
    pros::controller_analog_e_t analog = ANALOG_LEFT_Y;
    double threshold = 0.0;
    std::chrono::milliseconds sequence_window{500};
};

// Input mapper class
template<typename ChassisConfig>
class InputMapper : public core::ISubsystem {
private:
    std::string name_;
    pros::Controller& controller_;
    std::unordered_map<std::string, InputBinding> bindings_;
    std::unordered_map<std::string, std::function<void()>> actions_;
    std::vector<std::pair<std::chrono::steady_clock::time_point, std::string>> input_history_;
    bool enabled_ = false;
    
    bool checkBinding(const InputBinding& binding) {
        switch (binding.type) {
            case InputType::BUTTON:
                return !binding.buttons.empty() && 
                       controller_.get_digital_new_press(binding.buttons[0]); // Changed to new_press
                
            case InputType::BUTTON_COMBO: {
                return std::all_of(binding.buttons.begin(), binding.buttons.end(),
                    [this](auto btn) { return controller_.get_digital(btn); });
            }
            
            case InputType::ANALOG_ABOVE:
                return (controller_.get_analog(binding.analog) / 127.0) > binding.threshold;
                
            case InputType::ANALOG_BELOW:
                return (controller_.get_analog(binding.analog) / 127.0) < binding.threshold;
                
            case InputType::SEQUENCE: {
                auto now = std::chrono::steady_clock::now();
                
                // Clean old inputs
                input_history_.erase(
                    std::remove_if(input_history_.begin(), input_history_.end(),
                        [&](const auto& entry) {
                            return now - entry.first > binding.sequence_window;
                        }),
                    input_history_.end()
                );

                // Check sequence
                if (input_history_.size() == binding.buttons.size()) {
                    bool matches = true;
                    for (size_t i = 0; i < binding.buttons.size(); i++) {
                        if (controller_.get_digital(binding.buttons[i]) != 
                            controller_.get_digital(binding.buttons[i])) {
                            matches = false;
                            break;
                        }
                    }
                    if (matches) {
                        input_history_.clear();
                        return true;
                    }
                }

                // Record new input
                for (auto btn : binding.buttons) {
                    if (controller_.get_digital_new_press(btn)) {
                        input_history_.emplace_back(now, std::to_string(static_cast<int>(btn)));
                        break;
                    }
                }
                return false;
            }
        }
        return false;
    }

public:
    InputMapper(const std::string& name, pros::Controller& controller) 
        : name_(name), controller_(controller) {}

    // ISubsystem interface implementation
    void initialize() override { enabled_ = true; }
    void update() override {
        if (!enabled_) return;
        
        for (const auto& [name, binding] : bindings_) {
            if (checkBinding(binding)) {
                if (auto it = actions_.find(name); it != actions_.end()) {
                    it->second();
                }
            }
        }
    }
    void disable() override { enabled_ = false; }
    bool isEnabled() const override { return enabled_; }
    const std::string& getName() const override { return name_; }

    void addBinding(const std::string& name, const InputBinding& binding, 
                   std::function<void()> action) {
        bindings_[name] = binding;
        actions_[name] = std::move(action);
    }

    void removeBinding(const std::string& name) {
        bindings_.erase(name);
        actions_.erase(name);
    }
};

// Macro system class
template<typename ChassisConfig>
class MacroSystem : public core::ISubsystem {
private:
    std::string name_;
    Chassis<ChassisConfig>& chassis_;
    std::unordered_map<std::string, std::unique_ptr<Macro>> macros_;
    std::string active_macro_;
    bool enabled_ = false;

public:
    MacroSystem(const std::string& name, Chassis<ChassisConfig>& chassis) 
        : name_(name), chassis_(chassis) {}

    // ISubsystem interface implementation
    void initialize() override { enabled_ = true; }
    void update() override {
        if (!enabled_ || active_macro_.empty()) return;
        
        if (auto it = macros_.find(active_macro_); it != macros_.end()) {
            it->second->execute();
            if (it->second->isComplete()) {
                stopMacro();
            }
        }
    }
    void disable() override { 
        enabled_ = false;
        stopMacro();
    }
    bool isEnabled() const override { return enabled_; }
    const std::string& getName() const override { return name_; }

    void registerMacro(const std::string& name, std::unique_ptr<Macro> macro) {
        macros_[name] = std::move(macro);
    }

    bool startMacro(const std::string& name) {
        if (!enabled_) return false;
        
        if (auto it = macros_.find(name); it != macros_.end() && active_macro_.empty()) {
            active_macro_ = name;
            it->second->reset();
            return true;
        }
        return false;
    }

    void stopMacro() {
        active_macro_.clear();
    }

    bool isMacroActive() const {
        return !active_macro_.empty();
    }

    Chassis<ChassisConfig>& getChassis() { return chassis_; }
};

// Movement macro implementation
class MovementMacro : public Macro {
private:
    std::function<void()> movement_func_;
    bool complete_ = false;

public:
    explicit MovementMacro(std::function<void()> func) 
        : movement_func_(std::move(func)) {}

    void execute() override {
        if (!complete_) {
            movement_func_();
            complete_ = true;
        }
    }

    bool isComplete() const override {
        return complete_;
    }

    void reset() override {
        complete_ = false;
    }
};

// Enhanced driver control
template<typename ChassisConfig>
class EnhancedDriverControl : public core::ISubsystem {
private:
    std::string name_;
    MacroSystem<ChassisConfig>& macro_system_;
    InputMapper<ChassisConfig>& input_mapper_;
    DriverConfig config_;
    bool enabled_ = false;

public:
    EnhancedDriverControl(const std::string& name,
                         MacroSystem<ChassisConfig>& macro_system,
                         InputMapper<ChassisConfig>& input_mapper,
                         const DriverConfig& config = DriverConfig())
        : name_(name)
        , macro_system_(macro_system)
        , input_mapper_(input_mapper)
        , config_(config) {}

    // ISubsystem interface implementation
    void initialize() override { enabled_ = true; }
    void update() override {
        if (!enabled_) return;
        
        if (macro_system_.isMacroActive()) {
            macro_system_.update();
        } else {
            input_mapper_.update();
        }
    }
    void disable() override { 
        enabled_ = false;
        macro_system_.disable();
    }
    bool isEnabled() const override { return enabled_; }
    const std::string& getName() const override { return name_; }
};

} // namespace movement
