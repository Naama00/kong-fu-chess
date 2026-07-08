// Repository: https://github.com/Naama00/kong-fu-chess.git

#include <cassert>
#include <memory>
#include "board/Board.hpp"
#include "game/Game.hpp"
#include "pieces/King.hpp"
#include "pieces/Rook.hpp"

int main() {
    // --- Basic state machine ---
    kungfu::Game game;
    assert(!game.isRunning());
    assert(!game.isFinished());
    assert(!game.tryMove(kungfu::Position(0, 0), kungfu::Position(1, 1)));

    game.start();
    assert(game.isRunning());

    game.stop();
    assert(!game.isRunning());
    assert(!game.tryMove(kungfu::Position(0, 0), kungfu::Position(1, 1)));

    const auto beforeStart = kungfu::Game{};
    assert(!beforeStart.isRunning());

    // --- Capturing enemy king ends the game ---
    {
        auto board = std::make_shared<kungfu::Board>();
        auto whiteKing = std::make_shared<kungfu::King>(kungfu::PlayerColor::White, kungfu::Position(0, 0));
        auto blackKing = std::make_shared<kungfu::King>(kungfu::PlayerColor::Black, kungfu::Position(1, 1));
        board->placePiece(whiteKing, kungfu::Position(0, 0));
        board->placePiece(blackKing, kungfu::Position(1, 1));

        kungfu::Game g(board);
        g.start();
        assert(g.isRunning());

        const bool captured = g.tryMove(kungfu::Position(0, 0), kungfu::Position(1, 1));
        assert(captured);
        assert(g.isFinished());
        assert(!g.isRunning());
    }

    // --- After game over, further moves are ignored ---
    {
        auto board = std::make_shared<kungfu::Board>();
        auto whiteKing = std::make_shared<kungfu::King>(kungfu::PlayerColor::White, kungfu::Position(0, 0));
        auto blackKing = std::make_shared<kungfu::King>(kungfu::PlayerColor::Black, kungfu::Position(1, 1));
        auto whiteRook = std::make_shared<kungfu::Rook>(kungfu::PlayerColor::White, kungfu::Position(7, 7));
        board->placePiece(whiteKing, kungfu::Position(0, 0));
        board->placePiece(blackKing, kungfu::Position(1, 1));
        board->placePiece(whiteRook, kungfu::Position(7, 7));

        kungfu::Game g(board);
        g.start();
        g.tryMove(kungfu::Position(0, 0), kungfu::Position(1, 1));
        assert(g.isFinished());
        assert(!g.tryMove(kungfu::Position(7, 7), kungfu::Position(7, 0)));
    }

    return 0;
}
