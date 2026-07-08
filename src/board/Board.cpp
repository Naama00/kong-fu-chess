#include "board/Board.hpp"

#include <algorithm>

namespace kungfu {

Board::Board() = default;

std::optional<PiecePtr> Board::pieceAt(const Position& position) const {
    for (const auto& piece : pieces_) {
        if (piece && piece->position() == position) {
            return piece;
        }
    }
    return std::nullopt;
}

bool Board::placePiece(const PiecePtr& piece, const Position& position) {
    if (!piece) {
        return false;
    }

    if (pieceAt(position).has_value()) {
        return false;
    }

    piece->setPosition(position);
    pieces_.push_back(piece);
    return true;
}

bool Board::removePiece(const Position& position) {
    auto it = std::find_if(pieces_.begin(), pieces_.end(), [&](const PiecePtr& piece) {
        return piece && piece->position() == position;
    });

    if (it == pieces_.end()) {
        return false;
    }

    pieces_.erase(it);
    return true;
}

bool Board::movePiece(const Position& from, const Position& to) {
    auto it = std::find_if(pieces_.begin(), pieces_.end(), [&](const PiecePtr& piece) {
        return piece && piece->position() == from;
    });

    if (it == pieces_.end()) {
        return false;
    }

    if (pieceAt(to).has_value()) {
        removePiece(to);
    }

    (*it)->setPosition(to);
    return true;
}

std::vector<PiecePtr> Board::pieces() const {
    return pieces_;
}

}  // namespace kungfu
