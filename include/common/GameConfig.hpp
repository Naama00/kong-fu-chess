// include/common/GameConfig.hpp
#pragma once

namespace kungfu {

struct GameConfig {
    static constexpr int kBoardSize = 8;
    static constexpr int kDefaultCellSize = 100;

    int cooldownDurationMs = 2000;
    int msPerCellSpeed = 1000;
    int jumpDurationMs = 1000;

    bool allowSimultaneousMovement = true;
    bool enablePremoves = true;
};

}  // namespace kungfu