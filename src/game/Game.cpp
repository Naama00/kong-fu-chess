#include "game/Game.hpp"

#include "pieces/Queen.hpp"

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

bool Game::isFinished() const {
    return state_ == GameState::Finished;
}

bool Game::tryMove(const Position& from, const Position& to) {
    if (state_ != GameState::Running) {
        return false;
    }

    if (!ruleEngine_->isValidMove(from, to)) {
        return false;
    }

    // A moving piece cannot initiate a second move.
    const auto sourcePiece = board_->pieceAt(from);
    if (sourcePiece.has_value() && sourcePiece.value()->state() == PieceState::Moving) {
        return false;
    }

    if (collisionSystem_->isFriendlyBlock(from, to)) {
        return false;
    }

    // For a double-step pawn move, the intermediate square must be empty.
    const auto middle = movementSystem_.pawnDoubleStepMiddle(from, to);
    if (middle.has_value() && !collisionSystem_->isPathClear(from, to)) {
        return false;
    }

    // If the destination holds an airborne friendly piece, it blocks the move.
    const auto targetPiece = board_->pieceAt(to);
    if (targetPiece.has_value() && targetPiece.value()->isAirborne() &&
        targetPiece.value()->color() == sourcePiece.value()->color()) {
        return false;
    }

    // If an enemy piece is airborne at the destination, it captures the arriving piece.
    if (targetPiece.has_value() && targetPiece.value()->isAirborne() &&
        targetPiece.value()->color() != sourcePiece.value()->color()) {
        // Airborne captures: remove the arriving piece, airborne stays.
        sourcePiece.value()->setState(PieceState::Captured);
        board_->removePiece(from);
        return true;
    }

    // Record whether we are capturing a king before movePiece removes it.
    const bool capturedKing = collisionSystem_->isCapture(from, to) &&
                              targetPiece.has_value() &&
                              targetPiece.value()->type() == PieceType::King;

    if (!board_->movePiece(from, to)) {
        return false;
    }

    if (capturedKing) {
        state_ = GameState::Finished;
        return true;
    }

    // Delegate promotion decision and piece replacement to the rule engine.
    const auto movedPiece = board_->pieceAt(to);
    if (movedPiece.has_value() &&
        movedPiece.value()->type() == PieceType::Pawn &&
        ruleEngine_->isPawnPromotion(to, movedPiece.value()->color())) {
        board_->replacePiece(to, std::make_shared<Queen>(movedPiece.value()->color(), to));
    }

    return true;
}

bool Game::tryJump(const Position& cell) {
    if (state_ != GameState::Running) {
        return false;
    }

    const auto piece = board_->pieceAt(cell);
    if (!piece.has_value() || !piece.value()) {
        return false;
    }

    const PieceState pieceState = piece.value()->state();
    // A moving or already-airborne or captured piece cannot jump.
    if (pieceState == PieceState::Moving ||
        pieceState == PieceState::Airborne ||
        pieceState == PieceState::Captured) {
        return false;
    }

    piece.value()->setState(PieceState::Airborne);
    return true;
}

void Game::resolveJump(const Position& cell) {
    const auto piece = board_->pieceAt(cell);
    if (!piece.has_value() || !piece.value()->isAirborne()) {
        return;
    }
    // No enemy arrived — piece lands normally (state back to Idle).
    piece.value()->setState(PieceState::Idle);
}

bool Game::handleArrivalAtAirbornCell(const Position& cell, const Position& arrivingFrom) {
    const auto airbornePiece = board_->pieceAt(cell);
    if (!airbornePiece.has_value() || !airbornePiece.value()->isAirborne()) {
        return false;
    }

    const auto arrivingPiece = board_->pieceAt(arrivingFrom);
    if (!arrivingPiece.has_value()) {
        return false;
    }

    // Same color — no capture.
    if (airbornePiece.value()->color() == arrivingPiece.value()->color()) {
        return false;
    }

    // Airborne captures the arriving enemy: remove it, airborne piece stays.
    board_->removePiece(arrivingFrom);
    airbornePiece.value()->setState(PieceState::Idle);

    if (arrivingPiece.value()->type() == PieceType::King) {
        state_ = GameState::Finished;
    }

    return true;
}

}  // namespace kungfu
