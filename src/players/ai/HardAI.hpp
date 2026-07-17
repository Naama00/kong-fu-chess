// players/ai/HardAI.hpp
#pragma once

#include <cstdint>
#include <random>
#include <vector>

#include "players/IPlayer.hpp"
#include "engine/actions/ActionRequest.hpp"
#include "engine/analysis/MoveGenerator.hpp"
#include "engine/snapshot/GameSnapshot.hpp"

namespace kungfu {

class HardAI : public IPlayer {
public:
    explicit HardAI(PlayerColor playerColor, std::mt19937::result_type seed = 0xC0FFEEu, int searchDepth = 2);
    ~HardAI() override = default;

    std::vector<ActionRequest> decideActions(const view::GameSnapshot& snapshot) override;

private:
    int minimax(const view::GameSnapshot& snapshot, int depth, int alpha, int beta, bool maximizingPlayer) const;

    PlayerColor playerColor_;
    std::mt19937 rng_;
    MoveGenerator moveGenerator_;
    int searchDepth_;
    std::uint64_t nextRequestId_ = 1;
};

} // namespace kungfu