#pragma once

#include <memory>
#include "board/Board.hpp"
#include "board/IBoard.hpp"
#include "collision/CollisionSystem.hpp"
#include "common/GameState.hpp"
#include "common/Position.hpp"
#include "movement/MovementSystem.hpp"
#include "pieces/Piece.hpp"
#include "rules/RuleEngine.hpp"

namespace kungfu {

class Game {
public:
    Game();
    explicit Game(BoardPtr board);

    void start();
    void stop();
    bool isRunning() const;

    bool tryMove(const Position& from, const Position& to);

private:
    GameState state_;
    BoardPtr board_;
    std::shared_ptr<RuleEngine> ruleEngine_;
    std::shared_ptr<CollisionSystem> collisionSystem_;
    MovementSystem movementSystem_;
};

}  // namespace kungfu
