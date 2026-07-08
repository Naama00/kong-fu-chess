#include <cassert>
#include <memory>
#include "board/Board.hpp"
#include "collision/CollisionSystem.hpp"
#include "pieces/King.hpp"

int main() {
    auto board = std::make_shared<kungfu::Board>();
    auto attacker = std::make_shared<kungfu::King>(kungfu::PlayerColor::White, kungfu::Position(0, 0));
    auto defender = std::make_shared<kungfu::King>(kungfu::PlayerColor::Black, kungfu::Position(1, 1));

    board->placePiece(attacker, kungfu::Position(0, 0));
    board->placePiece(defender, kungfu::Position(1, 1));

    kungfu::CollisionSystem collision(board);
    const auto collisionPiece = collision.findCollision(kungfu::Position(0, 0), kungfu::Position(1, 1));

    assert(collisionPiece.has_value());
    assert(collisionPiece.value() == defender);

    return 0;
}
