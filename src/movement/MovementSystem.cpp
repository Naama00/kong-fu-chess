#include "movement/MovementSystem.hpp"

#include <cmath>

namespace kungfu {

namespace {
constexpr int kBoardSize = 8;
}

bool MovementSystem::isInBounds(const Position& position) const {
    return position.row() >= 0 && position.row() < kBoardSize &&
           position.col() >= 0 && position.col() < kBoardSize;
}

bool MovementSystem::isSamePosition(const Position& from, const Position& to) const {
    return from == to;
}

bool MovementSystem::canMoveTo(const Position& from, const Position& to) const {
    return isInBounds(from) && isInBounds(to) && !isSamePosition(from, to);
}

bool MovementSystem::isValidMove(const Piece& piece, const Position& from, const Position& to) const {
    if (!canMoveTo(from, to)) {
        return false;
    }

    const int rowDelta = std::abs(to.row() - from.row());
    const int colDelta = std::abs(to.col() - from.col());

    switch (piece.type()) {
        case PieceType::King:
            return (rowDelta <= 1 && colDelta <= 1);
        case PieceType::Queen:
            return (rowDelta == 0 || colDelta == 0 || rowDelta == colDelta);
        case PieceType::Rook:
            return (rowDelta == 0 || colDelta == 0);
        case PieceType::Bishop:
            return (rowDelta == colDelta);
        case PieceType::Knight:
            return (rowDelta == 2 && colDelta == 1) || (rowDelta == 1 && colDelta == 2);
        case PieceType::Pawn:
            return (piece.color() == PlayerColor::White) ? (to.row() == from.row() + 1 && colDelta == 0)
                                                       : (to.row() == from.row() - 1 && colDelta == 0);
    }

    return false;
}

}  // namespace kungfu
