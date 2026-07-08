#include "game/Game.hpp"

namespace kungfu {

Game::Game() : Game(std::make_shared<Board>()) {}

Game::Game(BoardPtr board) : state_(GameState::NotStarted), board_(std::move(board)), ruleEngine_(std::make_shared<RuleEngine>(board_)), collisionSystem_(std::make_shared<CollisionSystem>(board_)) {}

void Game::start() {
    state_ = GameState::Running;
}

void Game::stop() {
    state_ = GameState::Paused;
}

bool Game::isRunning() const {
    return state_ == GameState::Running;
}

bool Game::tryMove(const Position& from, const Position& to) {
    if (state_ != GameState::Running) {
        return false;
    }

    if (!ruleEngine_->isValidMove(from, to)) {
        return false;
    }

    const auto collisionPiece = collisionSystem_->findCollision(from, to);
    if (collisionPiece.has_value()) {
        return false;
    }

    return board_->movePiece(from, to);
}

}  // namespace kungfu
