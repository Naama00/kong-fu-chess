#include "engine/GameEngine.hpp"
#include <cmath>
#include <algorithm>

namespace kungfu {

GameEngine::GameEngine(std::shared_ptr<IBoard> board, std::shared_ptr<RuleEngine> ruleEngine) noexcept
    : board_(std::move(board)), ruleEngine_(std::move(ruleEngine)), arbiter_(board_) {}

MoveResult GameEngine::requestMove(const Position& from, const Position& to) {
    if (gameOver_) {
        return {false, "game_over"};
    }

    auto pieceOpt = board_->pieceAt(from);
    if (!pieceOpt.has_value() || !pieceOpt.value()) {
        return {false, "empty_source"};
    }

    auto piece = pieceOpt.value();

    // 1. מניעת תנועה של כלי שכבר נמצא כרגע בתנועה פעילה
    if (arbiter_.isPieceMoving(piece)) {
        return {false, "piece_already_moving"};
    }

    // 2. בדיקה האם הכלי נמצא כרגע בזמן צינון (Cooldown)
    if (arbiter_.isOnCooldown(piece, currentTimeMs_)) {
        return {false, "piece_on_cooldown"};
    }

    // 3. בדיקת חוקיות מול RuleEngine
    auto validation = ruleEngine_->validateMove(from, to);
    if (!validation.isValid) {
        return {false, validation.reason};
    }

    // חישוב מרחק תנועה דטרמיניסטי (Chebyshev Distance)
    int rowDelta = std::abs(to.row() - from.row());
    int colDelta = std::abs(to.col() - from.col());
    int distance = std::max(rowDelta, colDelta);
    int durationMs = distance * 1000;

    // תחילת התנועה
    piece->setState(PieceState::Moving);
    arbiter_.startMotion(piece, from, to, currentTimeMs_, durationMs);

    return {true, "ok"};
}

bool GameEngine::hasPieceAt(const Position& pos) const {
    if (!board_) return false;
    return board_->pieceAt(pos).has_value();
}

int GameEngine::getBoardRows() const {
    return board_ ? board_->rows() : 0;
}

int GameEngine::getBoardCols() const {
    return board_ ? board_->cols() : 0;
}

void GameEngine::wait(int ms) noexcept {
    if (gameOver_ || ms <= 0) {
        return;
    }

    // קידום הזמן וקבלת אירועי הגעה מהבורר
    auto arrivalEvents = arbiter_.advanceTime(ms, currentTimeMs_);
    
    for (const auto& event : arrivalEvents) {
        if (event.capturedKing) {
            gameOver_ = true;
        }
    }
}

}  // namespace kungfu