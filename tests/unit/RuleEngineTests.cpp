#include <cassert>
#include <memory>
#include "rules/RuleEngine.hpp"

int main() {
    auto board = std::make_shared<kungfu::Board>();
    auto king = std::make_shared<kungfu::Piece>(kungfu::PieceType::King, kungfu::PlayerColor::White, kungfu::Position(0, 0));
    board->placePiece(king, kungfu::Position(0, 0));

    kungfu::RuleEngine engine(board);

    assert(engine.isValidMove(kungfu::Position(0, 0), kungfu::Position(1, 1)));
    assert(!engine.isValidMove(kungfu::Position(0, 0), kungfu::Position(0, 0)));

    return 0;
}
