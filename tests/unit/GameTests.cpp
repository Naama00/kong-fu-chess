#include <cassert>
#include <memory>
#include "game/Game.hpp"
#include "pieces/King.hpp"

int main() {
    kungfu::Game game;
    game.start();

    auto king = std::make_shared<kungfu::King>(kungfu::PlayerColor::White, kungfu::Position(0, 0));
    game.tryMove(kungfu::Position(0, 0), kungfu::Position(1, 1));

    assert(game.isRunning());

    return 0;
}
