#pragma once

#include <memory>
#include <optional>
#include "board/IBoard.hpp"
#include "common/Position.hpp"
#include "pieces/Piece.hpp"

namespace kungfu {

class CollisionSystem {
public:
    explicit CollisionSystem(BoardPtr board);

    std::optional<PiecePtr> findCollision(const Position& from, const Position& to) const;

private:
    BoardPtr board_;
};

}  // namespace kungfu
