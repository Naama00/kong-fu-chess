#pragma once

#include "engine/board/Piece.hpp"
#include "engine/board/IBoard.hpp"
#include "engine/common/Position.hpp"

namespace kungfu {

class IPromotionRule {
public:
    virtual ~IPromotionRule() = default;

    // Checks whether the given piece is eligible for crowning at the target location, and if necessary
    // Performs the actual replacement on the board and returns the new piece.
    // If there is no crowning - returns the same piece that was received unchanged.
    virtual PiecePtr maybePromote(const PiecePtr& piece, const Position& to, IBoard& board) const = 0;
};

}  // namespace kungfu