#include "collision/CollisionSystem.hpp"

namespace kungfu {

CollisionSystem::CollisionSystem(BoardPtr board) : board_(std::move(board)) {}

std::optional<PiecePtr> CollisionSystem::findCollision(const Position& from, const Position& to) const {
    if (!board_) {
        return std::nullopt;
    }

    const auto targetPiece = board_->pieceAt(to);
    if (targetPiece.has_value()) {
        return targetPiece;
    }

    const auto sourcePiece = board_->pieceAt(from);
    if (sourcePiece.has_value()) {
        return std::nullopt;
    }

    return std::nullopt;
}

}  // namespace kungfu