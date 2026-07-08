#include "rules/RuleEngine.hpp"

namespace kungfu {

RuleEngine::RuleEngine(BoardPtr board) : board_(std::move(board)) {}

bool RuleEngine::isValidMove(const Position& from, const Position& to) const {
    if (!board_) {
        return false;
    }

    if (!movementSystem_.canMoveTo(from, to)) {
        return false;
    }

    const auto sourcePiece = board_->pieceAt(from);
    if (!sourcePiece.has_value() || !sourcePiece.value() || !sourcePiece.value()->isMovable()) {
        return false;
    }

    return true;
}

}  // namespace kungfu
