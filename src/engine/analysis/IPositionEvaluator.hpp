// engine/analysis/IPositionEvaluator.hpp
#pragma once

#include "engine/snapshot/GameSnapshot.hpp"
#include "engine/common/Enums.hpp"

namespace kungfu {

class IPositionEvaluator {
public:
    virtual ~IPositionEvaluator() = default;

    virtual int evaluate(const view::GameSnapshot& snapshot, PlayerColor playerColor) const = 0;
};

} // namespace kungfu