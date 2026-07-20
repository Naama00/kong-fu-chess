#pragma once

#include <vector>
#include <optional>
#include "engine/realtime/Motion.hpp"
#include "engine/common/GameConfig.hpp"

namespace kungfu {

struct MidRouteCollision {
    Motion winner;
    Motion loser;
};

// The CollisionDetector class is responsible for detecting mid-route collisions and arrivals of pieces in motion.
// It provides static methods to analyze the current state of active motions and determine if any collisions or
// arrivals have occurred based on the game configuration and current time.
class CollisionDetector {
public:
    static std::vector<MidRouteCollision> detectMidRouteCollisions(
        const std::vector<Motion>& activeMotions,
        const GameConfig& config
    ) noexcept;

    static std::vector<Motion> detectArrivals(
        const std::vector<Motion>& activeMotions,
        int currentTimeMs
    ) noexcept;

private:
    // Internal helper method for performing detailed (Narrow Phase) collision checking between two trajectories
    static std::optional<MidRouteCollision> checkDetailedCollision(
        const Motion& m1, 
        const Motion& m2
    ) noexcept;
};

} // namespace kungfu