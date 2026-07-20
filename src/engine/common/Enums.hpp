#pragma once

namespace kungfu {

enum class PieceType {
    King,
    Queen,
    Rook,
    Bishop,
    Knight,
    Pawn
};

enum class PlayerColor {
    White,
    Black
};

enum class PieceState {
    Idle,       // On the board, not moving
    Moving,     // Moving towards a destination
    Captured,   // Removed from the game
    Airborne    // Piece in the air during a jump
};

}  // namespace kungfu