// include/movement/tank_chassis.hpp

#pragma once
#include "movement/chassis.hpp"

namespace movement {

template<typename Config>
class TankChassis : public Chassis<Config> {
private:
    using Base = Chassis<Config>;
    using Base::motors_;
    using Base::enabled_;
    using Base::current_pos_;
    using Base::imu_;

    std::unique_ptr<pros::Rotation> left_encoder_;
    std::unique_ptr<pros::Rotation> right_encoder_;

    // PID Constants
    static constexpr double kP = 0.8;
    static constexpr double kI = 0.001;
    static constexpr double kD = 0.2;
    static constexpr double kTurnP = 1.2;

public:
    explicit TankChassis(const std::string& name = "tank_chassis") 
        : Base(name) {}

    void initializeSensors(int imu_port, int left_enc_port = -1, int right_enc_port = -1) override {
        Base::initializeSensors(imu_port);
        
        if constexpr (Config::odomType == OdomType::TRACKING) {
            if (left_enc_port != -1) {
                left_encoder_ = std::make_unique<pros::Rotation>(left_enc_port);
                left_encoder_->reset_position();
            }
            if (right_enc_port != -1) {
                right_encoder_ = std::make_unique<pros::Rotation>(right_enc_port);
                right_encoder_->reset_position();
            }
        }
    }

    void moveTo(const field::Point& target, bool reverse = false) override {
        if (!enabled_) return;

        while (enabled_) {
            Position current = this->getPosition();
            double distance = current.distanceTo(target);
            
            if (distance < 1.0) break; // 1 inch tolerance
            
            double angle_error = current.angleTo(target) - current.heading;
            if (reverse) angle_error += M_PI;
            
            // Normalize angle error
            while (angle_error > M_PI) angle_error -= 2*M_PI;
            while (angle_error < -M_PI) angle_error += 2*M_PI;
            
            // Calculate motor powers using PID
            double turn_power = kTurnP * angle_error;
            double drive_power = kP * distance;
            
            // Apply powers to motors
            for (size_t i = 0; i < motors_.size(); i++) {
                bool is_left = i < motors_.size()/2;
                double power = drive_power + (is_left ? turn_power : -turn_power);
                motors_[i].move_velocity(power * 200); // Scale to velocity
            }
            
            pros::delay(10);
        }
        
        this->stop();
    }

    void turnTo(double angle) override {
        if (!enabled_) return;

        while (enabled_) {
            double current = this->getPosition().heading;
            double error = angle - current;
            
            // Normalize error
            while (error > M_PI) error -= 2*M_PI;
            while (error < -M_PI) error += 2*M_PI;
            
            if (std::abs(error) < 0.05) break; // ~3 degree tolerance
            
            double power = kTurnP * error;
            
            // Apply powers to motors
            for (size_t i = 0; i < motors_.size(); i++) {
                bool is_left = i < motors_.size()/2;
                motors_[i].move_velocity((is_left ? power : -power) * 200);
            }
            
            pros::delay(10);
        }
        
        this->stop();
    }

    Position getPosition() const override {
        if constexpr (Config::odomType == OdomType::IMU_ENHANCED) {
            if (imu_) {
                double heading = imu_->get_heading() * M_PI / 180.0;
                return Position(this->current_pos_.x, this->current_pos_.y, heading);
            }
        }
        else if constexpr (Config::odomType == OdomType::TRACKING) {
            if (imu_ && left_encoder_ && right_encoder_) {
                double left_dist = left_encoder_->get_position() / 360.0 * (2.75 * M_PI);
                double right_dist = right_encoder_->get_position() / 360.0 * (2.75 * M_PI);
                double heading = imu_->get_heading() * M_PI / 180.0;
                
                double distance = (left_dist + right_dist) / 2.0;
                double new_x = this->current_pos_.x + distance * std::cos(heading);
                double new_y = this->current_pos_.y + distance * std::sin(heading);
                
                this->current_pos_.x = new_x;
                this->current_pos_.y = new_y;
                this->current_pos_.heading = heading;
            }
        }
        return this->current_pos_;
    }
};

} // namespace movement
