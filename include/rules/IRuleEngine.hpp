#pragma once

#include "common/Enums.hpp"
#include "common/Position.hpp"

namespace kungfu {

class IRuleEngine {
public:
    virtual ~IRuleEngine() = default;
    virtual bool isValidMove(const Position& from, const Position& to) const = 0;

    // Returns true if a pawn at 'to' with the given color should be promoted.
    virtual bool isPawnPromotion(const Position& to, PlayerColor color) const = 0;
};

}  // namespace kungfu
