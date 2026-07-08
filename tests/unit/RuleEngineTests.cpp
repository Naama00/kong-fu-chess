// Repository: https://github.com/Naama00/kong-fu-chess.git

#include <cassert>
#include <memory>
#include "board/Board.hpp"
#include "pieces/King.hpp"
#include "rules/RuleEngine.hpp"

int main() {
    auto board = std::make_shared<kungfu::Board>();
    auto king = std::make_shared<kungfu::King>(kungfu::PlayerColor::White, kungfu::Position(0, 0));
    board->placePiece(king, kungfu::Position(0, 0));

    kungfu::RuleEngine engine(board);

    assert(engine.isValidMove(kungfu::Position(0, 0), kungfu::Position(1, 1)));
    assert(!engine.isValidMove(kungfu::Position(0, 0), kungfu::Position(0, 0)));
    assert(!engine.isValidMove(kungfu::Position(0, 0), kungfu::Position(2, 2)));
    assert(!engine.isValidMove(kungfu::Position(9, 9), kungfu::Position(8, 8)));

    auto emptyBoard = std::make_shared<kungfu::Board>();
    kungfu::RuleEngine emptyEngine(emptyBoard);
    assert(!emptyEngine.isValidMove(kungfu::Position(0, 0), kungfu::Position(1, 1)));

    return 0;
}
