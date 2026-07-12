#include "board/Piece.hpp"

namespace kungfu {

std::uint64_t Piece::nextId_ = 1;

Piece::Piece(PieceType type, PlayerColor color, Position position)
    : type_(type), color_(color), position_(position), state_(PieceState::Idle), id_(nextId_++) {}

}  // namespace kungfu