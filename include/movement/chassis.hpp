#pragma once
#include "main.h"
#include "constants/fieldConstants.hpp"
#include "pros/motors.hpp"
#include "pros/imu.hpp"
#include "pros/rotation.hpp"
#include "core/subsystem.hpp"
#include <memory>
#include <vector>

namespace movement {

// Forward declarations
template<typename Config>
class Chassis;

// Configuration options
enum class DriveType {
    TANK,
    HOLONOMIC,
    MECANUM
};

enum class OdomType {
    NONE,           // Dead reckoning only
    TRACKING,       // Tracking wheels
    INTEGRATED,     // Integrated encoders
    IMU_ENHANCED    // IMU-enhanced tracking
};

// Base configuration struct
template<DriveType DT, OdomType OT>
struct ChassisConfig {
    static constexpr DriveType driveType = DT;
    static constexpr OdomType odomType = OT;
};

// Position tracking class
class Position {
public:
    double x;
    double y;
    double heading; // Radians

    Position(double x = 0, double y = 0, double heading = 0)
        : x(x), y(y), heading(heading) {}

    double distanceTo(const field::Point& target) const {
        double dx = target.x - x;
        double dy = target.y - y;
        return std::sqrt(dx*dx + dy*dy);
    }

    double angleTo(const field::Point& target) const {
        return std::atan2(target.y - y, target.x - x);
    }
};

// Base chassis class
template<typename Config = ChassisConfig<DriveType::TANK, OdomType::NONE>>
class Chassis : public core::ISubsystem {
protected:
    std::string name_;
    mutable Position current_pos_;
    std::vector<pros::Motor> motors_;
    std::unique_ptr<pros::IMU> imu_;
    bool enabled_ = false;

public:
    explicit Chassis(const std::string& name = "chassis") 
        : name_(name), current_pos_(), motors_(), imu_(nullptr) {}

    virtual ~Chassis() override = default;

    // ISubsystem interface implementation
    virtual void initialize() override { enabled_ = true; }
    virtual void update() override {}
    virtual void disable() override { 
        enabled_ = false;
        stop(); 
    }
    virtual bool isEnabled() const override { return enabled_; }
    virtual const std::string& getName() const override { return name_; }

    virtual void initializeSensors(int imu_port, int left_enc_port = -1, int right_enc_port = -1) {
        if constexpr (Config::odomType == OdomType::IMU_ENHANCED || Config::odomType == OdomType::TRACKING) {
            imu_ = std::make_unique<pros::IMU>(imu_port);
            if (imu_) {
                imu_->reset();
                pros::delay(2000); // Wait for IMU calibration
            }
        }
    }

    // Motor management
    virtual void addMotor(int port, bool reversed = false) {
        if (motors_.size() < 10) {
            pros::Motor motor(port);
            if (reversed) {
                motor.set_reversed(true);
            }
            motors_.push_back(motor);
        }
    }

    // Direct motor control
    virtual void setMotorVelocity(int index, double velocity) {
        if (index >= 0 && index < static_cast<int>(motors_.size())) {
            motors_[index].move_velocity(velocity);
        }
    }

    // Required movement interface
    virtual void moveTo(const field::Point& target, bool reverse = false) = 0;
    virtual void turnTo(double angle) = 0;
    virtual void stop() {
        for (auto& motor : motors_) {
            motor.move_velocity(0);
        }
    }

    // Position tracking
    virtual Position getPosition() const { return current_pos_; }

    // Accessors
    size_t getMotorCount() const { return motors_.size(); }
    const pros::Motor& getMotor(int index) const { 
        return motors_.at(index);
    }
};

} // namespace movement
