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
    bool isFinished() const;

    bool tryMove(const Position& from, const Position& to);

    // Marks the piece at 'cell' as airborne for 1000 ms.
    // Returns false if the piece is already moving, airborne, or the game is not running.
    bool tryJump(const Position& cell);

    // Must be called by the game loop to resolve the jump after 1000 ms.
    // If an enemy arrives at the airborne cell during that window, it is captured.
    // Otherwise the piece lands back (state → Idle).
    void resolveJump(const Position& cell);

    // Called when an enemy arrives at a cell occupied by an airborne piece.
    // Returns true if the airborne piece captured the arriving piece.
    bool handleArrivalAtAirbornCell(const Position& cell, const Position& arrivingFrom);

private:
    GameState state_;
    BoardPtr board_;
    std::shared_ptr<RuleEngine> ruleEngine_;
    std::shared_ptr<CollisionSystem> collisionSystem_;
    MovementSystem movementSystem_;
};

}  // namespace kungfu
