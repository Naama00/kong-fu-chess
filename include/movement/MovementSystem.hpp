#pragma once

#include <optional>
#include "common/Position.hpp"
#include "pieces/Piece.hpp"

namespace kungfu {

class MovementSystem {
public:
    bool isInBounds(const Position& position) const;
    bool isSamePosition(const Position& from, const Position& to) const;
    bool canMoveTo(const Position& from, const Position& to) const;
    bool isValidMove(const Piece& piece, const Position& from, const Position& to) const;

    // Returns the square a pawn passes through on a double-step move,
    // or std::nullopt if the move is not a pawn double-step.
    std::optional<Position> pawnDoubleStepMiddle(const Position& from, const Position& to) const;
};

}  // namespace kungfu
