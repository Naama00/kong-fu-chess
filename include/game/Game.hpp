#pragma once

#include <memory>
#include <optional>
#include <iostream>
#include "board/Board.hpp"
#include "board/IBoard.hpp"
#include "collision/CollisionSystem.hpp"
#include "common/GameState.hpp"
#include "common/Position.hpp"
#include "movement/MovementSystem.hpp"
#include "pieces/Piece.hpp"
#include "rules/RuleEngine.hpp"

namespace kungfu {

// מבנה פנימי לניהול תנועות מושהות בזמן אמת
struct PendingMove {
    Position from;
    Position to;
    int arrivalTimeMs;
};

class Game {
public:
    Game();
    explicit Game(BoardPtr board);

    void start();
    void stop();
    bool isRunning() const;
    bool isFinished() const;
    bool tryMove(const Position& from, const Position& to);
    void click(int x, int y);
    void wait(int ms);
    void printBoard(std::ostream& out) const;
    bool tryJump(const Position& cell);
    void resolveJump(const Position& cell);
    bool handleArrivalAtAirbornCell(const Position& cell, const Position& arrivingFrom);

private:
    std::string getPieceToken(const PiecePtr& piece) const;

    GameState state_;
    BoardPtr board_;
    std::shared_ptr<RuleEngine> ruleEngine_;
    std::shared_ptr<CollisionSystem> collisionSystem_;
    MovementSystem movementSystem_;

    std::optional<Position> selectedPosition_;
    std::optional<PendingMove> pendingMove_;
    int currentTimeMs_ = 0;
};

}  // namespace kungfu