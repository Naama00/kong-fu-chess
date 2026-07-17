// players/ai/MediumAI.cpp
#include "players/ai/MediumAI.hpp"
#include "engine/analysis/PositionEvaluator.hpp"
#include <algorithm>
#include <limits>

namespace kungfu {

namespace {

view::GameSnapshot simulateMove(const view::GameSnapshot& original, const Position& from, const Position& to, PlayerColor color) {
    view::GameSnapshot simulated = original;
    
    if (from != to) {
        for (auto& piece : simulated.pieces) {
            if (piece.logicalPosition == to && piece.color != color && piece.state != PieceState::Captured) {
                piece.state = PieceState::Captured;
                if (piece.type == PieceType::King) {
                    simulated.isGameOver = true;
                }
            }
        }
    }
    
    for (auto& piece : simulated.pieces) {
        if (piece.logicalPosition == from && piece.color == color && piece.state != PieceState::Captured) {
            piece.logicalPosition = to;
            piece.hasMoved = true;
            break;
        }
    }
    
    return simulated;
}

} // namespace

MediumAI::MediumAI(PlayerColor playerColor, std::mt19937::result_type seed)
    : playerColor_(playerColor), rng_(seed) {}

std::vector<ActionRequest> MediumAI::decideActions(const view::GameSnapshot& snapshot) {
    const auto legalMoves = moveGenerator_.generateForPlayer(snapshot, playerColor_);
    if (legalMoves.empty()) {
        return {};
    }

    int bestScore = std::numeric_limits<int>::min();
    std::vector<IMoveGenerator::MoveCandidate> bestMoves;
    PositionEvaluator evaluator;

    for (const auto& move : legalMoves) {
        auto simulated = simulateMove(snapshot, move.from, move.to, playerColor_);
        int score = evaluator.evaluate(simulated, playerColor_);

        if (score > bestScore) {
            bestScore = score;
            bestMoves.clear();
            bestMoves.push_back(move);
        } else if (score == bestScore) {
            bestMoves.push_back(move);
        }
    }

    if (bestMoves.empty()) {
        return {};
    }

    std::uniform_int_distribution<std::size_t> dist(0, bestMoves.size() - 1);
    const auto selected = bestMoves[dist(rng_)];

    return {ActionRequest(nextRequestId_++, playerColor_, PlayerAction(selected.from, selected.to))};
}

} // namespace kungfu