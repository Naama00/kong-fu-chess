// players/ai/MediumAI.hpp
#pragma once

#include <cstdint>
#include <random>
#include <vector>

#include "players/IPlayer.hpp"
#include "engine/actions/ActionRequest.hpp"
#include "engine/analysis/MoveGenerator.hpp"
#include "engine/snapshot/GameSnapshot.hpp"

namespace kungfu {

class MediumAI : public IPlayer {
public:
    explicit MediumAI(PlayerColor playerColor, std::mt19937::result_type seed = 0xC0FFEEu);
    ~MediumAI() override = default;

    std::vector<ActionRequest> decideActions(const view::GameSnapshot& snapshot) override;

private:
    PlayerColor playerColor_;
    std::mt19937 rng_;
    MoveGenerator moveGenerator_;
    std::uint64_t nextRequestId_ = 1;
};

} // namespace kungfu