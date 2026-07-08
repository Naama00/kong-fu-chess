#include "rules/RuleEngine.hpp"

#include "common/GameConfig.hpp"

namespace kungfu {

RuleEngine::RuleEngine(BoardPtr board) : board_(std::move(board)) {}

bool RuleEngine::isValidMove(const Position& from, const Position& to) const {
    if (!board_) {
        return false;
    }

    const auto sourcePiece = board_->pieceAt(from);
    if (!sourcePiece.has_value() || !sourcePiece.value() || !sourcePiece.value()->isMovable()) {
        return false;
    }

    // Validate both board bounds / same-square, and piece-specific movement geometry.
    return movementSystem_.isValidMove(*sourcePiece.value(), from, to);
}

bool RuleEngine::isPawnPromotion(const Position& to, PlayerColor color) const {
    return color == PlayerColor::White
               ? to.row() == GameConfig::kWhitePawnPromotionRow
               : to.row() == GameConfig::kBlackPawnPromotionRow;
}

}  // namespace kungfu
