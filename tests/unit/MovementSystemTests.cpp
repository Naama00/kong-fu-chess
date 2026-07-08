#include <cassert>
#include "movement/MovementSystem.hpp"
#include "pieces/Bishop.hpp"
#include "pieces/King.hpp"
#include "pieces/Knight.hpp"
#include "pieces/Pawn.hpp"
#include "pieces/Rook.hpp"

int main() {
    kungfu::MovementSystem movement;

    kungfu::King king(kungfu::PlayerColor::White, kungfu::Position(0, 0));
    assert(movement.isValidMove(king, kungfu::Position(0, 0), kungfu::Position(1, 1)));
    assert(!movement.isValidMove(king, kungfu::Position(0, 0), kungfu::Position(2, 2)));

    kungfu::Rook rook(kungfu::PlayerColor::White, kungfu::Position(3, 3));
    assert(movement.isValidMove(rook, kungfu::Position(3, 3), kungfu::Position(3, 7)));
    assert(!movement.isValidMove(rook, kungfu::Position(3, 3), kungfu::Position(4, 7)));

    kungfu::Bishop bishop(kungfu::PlayerColor::Black, kungfu::Position(2, 2));
    assert(movement.isValidMove(bishop, kungfu::Position(2, 2), kungfu::Position(5, 5)));
    assert(!movement.isValidMove(bishop, kungfu::Position(2, 2), kungfu::Position(5, 4)));

    kungfu::Knight knight(kungfu::PlayerColor::White, kungfu::Position(4, 4));
    assert(movement.isValidMove(knight, kungfu::Position(4, 4), kungfu::Position(2, 3)));
    assert(!movement.isValidMove(knight, kungfu::Position(4, 4), kungfu::Position(3, 3)));

    kungfu::Pawn pawn(kungfu::PlayerColor::White, kungfu::Position(1, 1));
    assert(movement.isValidMove(pawn, kungfu::Position(1, 1), kungfu::Position(2, 1)));
    assert(!movement.isValidMove(pawn, kungfu::Position(1, 1), kungfu::Position(0, 1)));

    return 0;
}
