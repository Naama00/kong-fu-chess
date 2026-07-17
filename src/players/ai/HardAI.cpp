// players/ai/HardAI.cpp
#include "players/ai/HardAI.hpp"
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

HardAI::HardAI(PlayerColor playerColor, std::mt19937::result_type seed, int searchDepth)
    : playerColor_(playerColor), rng_(seed), searchDepth_(searchDepth) {}

int HardAI::minimax(const view::GameSnapshot& snapshot, int depth, int alpha, int beta, bool maximizingPlayer) const {
    if (depth == 0 || snapshot.isGameOver) {
        PositionEvaluator evaluator;
        return evaluator.evaluate(snapshot, playerColor_);
    }

    PlayerColor opponentColor = (playerColor_ == PlayerColor::White) ? PlayerColor::Black : PlayerColor::White;
    PlayerColor movingColor = maximizingPlayer ? playerColor_ : opponentColor;

    auto legalMoves = moveGenerator_.generateForPlayer(snapshot, movingColor);
    if (legalMoves.empty()) {
        PositionEvaluator evaluator;
        return evaluator.evaluate(snapshot, playerColor_);
    }

    if (maximizingPlayer) {
        int maxEval = std::numeric_limits<int>::min();
        for (const auto& move : legalMoves) {
            auto simulated = simulateMove(snapshot, move.from, move.to, playerColor_);
            int eval = minimax(simulated, depth - 1, alpha, beta, false);
            maxEval = std::max(maxEval, eval);
            alpha = std::max(alpha, eval);
            if (beta <= alpha) {
                break;
            }
        }
        return maxEval;
    } else {
        int minEval = std::numeric_limits<int>::max();
        for (const auto& move : legalMoves) {
            auto simulated = simulateMove(snapshot, move.from, move.to, opponentColor);
            int eval = minimax(simulated, depth - 1, alpha, beta, true);
            minEval = std::min(minEval, eval);
            beta = std::min(beta, eval);
            if (beta <= alpha) {
                break;
            }
        }
        return minEval;
    }
}

std::vector<ActionRequest> HardAI::decideActions(const view::GameSnapshot& snapshot) {
    const auto legalMoves = moveGenerator_.generateForPlayer(snapshot, playerColor_);
    if (legalMoves.empty()) {
        return {};
    }

    int bestScore = std::numeric_limits<int>::min();
    std::vector<IMoveGenerator::MoveCandidate> bestMoves;

    for (const auto& move : legalMoves) {
        auto simulated = simulateMove(snapshot, move.from, move.to, playerColor_);
        int score = minimax(simulated, searchDepth_ - 1, std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), false);

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