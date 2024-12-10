// Field_Constants.hpp
#pragma once
#include "main.h"
#include <vector>

namespace field {

// Field dimensions (inches) - verified from page A-1
const double FIELD_WIDTH = 144.0;
const double FIELD_HEIGHT = 144.0;

// Base coordinate structures
struct Point {
    double x;
    double y;
    Point(double x, double y) : x(x), y(y) {}
};

struct FieldElement {
    Point position;
    double height;
    FieldElement(double x, double y, double h = 0) : position(x, y), height(h) {}
};

// Ladder Constants - verified from pages A-10 to A-12
namespace ladder {
    const FieldElement CENTER(72, 72, 0);
    const double BASE_WIDTH = 36.0;
    const double BASE_HEIGHT = 36.0;
    
    // Exact height levels
    const double LEVEL_1_HEIGHT = 18.16;
    const double LEVEL_2_HEIGHT = 32.16;
    const double LEVEL_3_HEIGHT = 46.16;
    const FieldElement HIGH_STAKE(72, 72, 46.16);
}

// Wall Stakes - ½" Schedule 40 PVC
namespace stakes {
    const double STAKE_HEIGHT = 14.5; // Height verification pending
    const FieldElement RED_ALLIANCE(72, 0, STAKE_HEIGHT);
    const FieldElement BLUE_ALLIANCE(72, 144, STAKE_HEIGHT);
    const FieldElement LEFT_NEUTRAL(0, 72, STAKE_HEIGHT);
    const FieldElement RIGHT_NEUTRAL(144, 72, STAKE_HEIGHT);
}

// Corners - verified 12"x12" triangular sections
namespace corners {
    struct Corner {
        Point position;
        bool isPositive;
        double size;
        Corner(double x, double y, bool pos) : 
            position(x, y), isPositive(pos), size(12.0) {}
    };

    const Corner BOTTOM_LEFT(0, 0, false);    // Negative
    const Corner BOTTOM_RIGHT(144, 0, true);  // Positive
    const Corner TOP_LEFT(0, 144, true);      // Positive
    const Corner TOP_RIGHT(144, 144, false);  // Negative
}

// Mobile Goals
namespace mobile_goals {
    const double GOAL_HEIGHT = 14.5;
    const double GOAL_DIAMETER = 10.0;
    
    // Starting positions pending final field specs
    const FieldElement BOTTOM_LEFT(36, 36, GOAL_HEIGHT);
    const FieldElement BOTTOM_RIGHT(108, 36, GOAL_HEIGHT);
    const FieldElement CENTER(72, 72, GOAL_HEIGHT);
    const FieldElement TOP_LEFT(36, 108, GOAL_HEIGHT);
    const FieldElement TOP_RIGHT(108, 108, GOAL_HEIGHT);
}

// Autonomous Line - positions to be verified with final specs
namespace auto_line {
    const double LOWER_Y = 60.0;
    const double UPPER_Y = 84.0;
}

// Starting Areas - defined as ranges along starting lines
namespace start_zones {
    struct StartZone {
        double y_pos;      // Y coordinate of starting line
        double x_min;      // Minimum legal X coordinate
        double x_max;      // Maximum legal X coordinate
    };
    
    const StartZone RED_ZONE = {18.0, 0.0, 144.0};
    const StartZone BLUE_ZONE = {126.0, 0.0, 144.0};
}

// Ring specifications
namespace rings {
    const double OUTER_DIAMETER = 7.0;
    const double INNER_DIAMETER = 3.0;
    const double THICKNESS = 2.0;
    
    // Starting positions pending final field specs
    // Each position represents bottom ring of a stack
    // Note: Some stacks flipped per Sept 3 update
    struct RingStack {
        Point position;
        int count;
        bool flipped;
        RingStack(double x, double y, int c, bool f) : 
            position(x, y), count(c), flipped(f) {}
    };

    // Ring stack positions to be updated when final specs released
    const std::vector<RingStack> RED_STACKS = {
        // Bottom row examples (to be verified)
        RingStack(24, 24, 4, false),
        RingStack(48, 24, 4, true),
        // Add remaining positions when verified
    };

    const std::vector<RingStack> BLUE_STACKS = {
        // To be populated with verified positions
    };
}

} // namespace field