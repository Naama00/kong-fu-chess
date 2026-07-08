#include <cassert>
#include <memory>
#include "board/Board.hpp"
#include "pieces/King.hpp"

int main() {
    kungfu::Board board;
    auto king = std::make_shared<kungfu::King>(kungfu::PlayerColor::White, kungfu::Position(0, 0));

    assert(board.placePiece(king, kungfu::Position(0, 0)));
    assert(board.pieceAt(kungfu::Position(0, 0)).has_value());
    assert(board.movePiece(kungfu::Position(0, 0), kungfu::Position(1, 1)));
    assert(!board.pieceAt(kungfu::Position(0, 0)).has_value());
    assert(board.pieceAt(kungfu::Position(1, 1)).has_value());

    return 0;
}
