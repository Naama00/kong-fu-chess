// src/board/Piece.cpp
#include "engine/board/Piece.hpp"
#include <atomic>

namespace kungfu {

// Define the class's static variable and allocate memory for it (starting at 1)
std::uint64_t Piece::nextId_ = 1;

Piece::Piece(PieceType type, PlayerColor color, Position position)
    : type_(type), color_(color), position_(position), state_(PieceState::Idle), id_(nextId_++) {}

}  // namespace kungfu