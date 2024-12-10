# Robot Documentation

## Overview
This robot is built using the PROS framework and features a tank drive chassis with a pneumatic clamp system. The robot supports multiple drive modes and includes both autonomous and operator control capabilities.

## Robot Configuration

### Hardware Setup
- **Chassis Motors**:
  - Left Side: Ports 11 (front) and 20 (back)
  - Right Side: Ports 1 (front) and 10 (back)
- **Sensors**:
  - IMU (Inertial Measurement Unit): Port 9
- **Pneumatics**:
  - Clamp Solenoid: Port 'B'

## Control System

### Drive Modes
The robot supports three different drive control modes:

1. **ARCADE** (Single Stick)
   - Left joystick controls both forward/backward movement and turning
   - Y-axis: Forward/Backward
   - X-axis: Turning

2. **SPLIT** (Default Mode)
   - Left joystick Y-axis: Forward/Backward
   - Right joystick X-axis: Turning
   - Provides more precise control over turning

3. **TANK**
   - Left joystick Y-axis: Left side motors
   - Right joystick Y-axis: Right side motors
   - Traditional tank drive control

### Control Features
- **Input Curve**: Implements a curve factor (default 1.5) for smoother control
- **Deadzone**: 5% deadzone to prevent drift
- **Turn Scaling**: 80% turn speed scaling for better control
- **Dual Controller Support**: Supports both primary (ID: 0) and partner (ID: 1) controllers

## Subsystems

### Chassis
- Tank drive configuration with 4 motors
- Enhanced IMU-based odometry for position tracking
- Supports both autonomous and driver control operations
- Velocity-based motor control (±200 units)

### Clamp
- Pneumatic control system
- Toggle functionality for grab/release
- Event system integration for state change notifications
- Supports development mode for testing without hardware

## Development Mode

### Features
- **Dev Mode Toggle**: Available for subsystems (especially clamp)
- **Event System**: Monitors state changes and system events
- **Subsystem Registry**: Centralized management of all robot subsystems

### Debug Capabilities
- Subsystem state monitoring
- Event tracking
- Hardware simulation in dev mode

## Autonomous Operation

### Default Autonomous Routine
1. Moves forward at 100 velocity for 1 second
2. Moves to specific field coordinates (24, 0)
3. Toggles clamp state

### Autonomous Features
- IMU-enhanced position tracking
- Point-to-point movement capabilities
- Macro system for complex autonomous routines
- Subsystem state management

## Competition Operation

### Initialization
- Configures all subsystems
- Sets up drive mode (default: SPLIT)
- Initializes sensors and motors

### Disabled State
- All subsystems automatically disabled
- Safe state management

### Driver Control
- 10ms control loop
- Real-time subsystem updates
- Smooth input processing

## Tips for Operation

1. **Starting Up**:
   - Ensure IMU is calibrated
   - Verify all motor connections
   - Check pneumatic system pressure

2. **During Operation**:
   - Monitor controller battery levels
   - Use appropriate drive mode for task
   - Utilize clamp toggle as needed

3. **Development**:
   - Use dev mode for testing without hardware
   - Monitor event system for debugging
   - Utilize subsystem registry for system management

## Common Issues & Solutions

1. **Drift Issues**:
   - Verify deadzone settings
   - Check IMU calibration
   - Inspect motor encoders

2. **Control Responsiveness**:
   - Adjust curve factor
   - Modify turn scaling
   - Check for system delays

3. **Clamp Problems**:
   - Verify pneumatic connections
   - Check solenoid functionality
   - Monitor system pressure
