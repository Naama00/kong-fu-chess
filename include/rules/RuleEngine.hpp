#pragma once

#include <memory>
#include "board/IBoard.hpp"
#include "common/Position.hpp"
#include "movement/MovementSystem.hpp"

namespace kungfu {

class RuleEngine {
public:
    explicit RuleEngine(BoardPtr board);

    bool isValidMove(const Position& from, const Position& to) const;

private:
    BoardPtr board_;
    MovementSystem movementSystem_;
};

}  // namespace kungfu
