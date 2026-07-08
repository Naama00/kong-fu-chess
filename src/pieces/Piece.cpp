#include "pieces/Piece.hpp"

namespace kungfu {

Piece::Piece(PieceType type, PlayerColor color, Position position)
    : type_(type), color_(color), position_(position) {}

PieceType Piece::type() const {
    return type_;
}

PlayerColor Piece::color() const {
    return color_;
}

Position Piece::position() const {
    return position_;
}

void Piece::setPosition(const Position& position) {
    position_ = position;
}

}  // namespace kungfu
