#pragma once

namespace kungfu {

// Configuration settings for the game engine, including movement speeds, cooldown durations, and gameplay options.
struct GameConfig {
    int cooldownDurationMs = 2000;
    int msPerCellSpeed = 1000;
    int jumpDurationMs = 1000;

    bool allowSimultaneousMovement = true;
    bool enablePremoves = true;
    bool allowJumping = true; 
};

}  // namespace kungfu