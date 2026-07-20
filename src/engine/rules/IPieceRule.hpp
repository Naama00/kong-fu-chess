#pragma once

#include <vector>
#include "engine/common/Position.hpp"
#include "engine/board/IBoard.hpp"
#include "engine/board/Piece.hpp"

namespace kungfu {

class IPieceRule {
public:
    virtual ~IPieceRule() = default;

    // Calculates and returns all possible geometric target coordinates for the given tool in the current board state.
    virtual std::vector<Position> getLegalDestinations(const IBoard& board, const Piece& piece) const = 0;
};

}  // namespace kungfu