// engine/analysis/ThreatAnalyzer.cpp
#include "engine/analysis/ThreatAnalyzer.hpp"

namespace kungfu {

std::vector<view::PieceSnapshot> ThreatAnalyzer::getThreateningPieces(
    const view::GameSnapshot& snapshot,
    const Position& targetPosition,
    PlayerColor defenderColor) const {
    
    std::vector<view::PieceSnapshot> threateningPieces;
    PlayerColor opponentColor = (defenderColor == PlayerColor::White) ? PlayerColor::Black : PlayerColor::White;
    
    MoveGenerator generator;
    auto opponentMoves = generator.generateForPlayer(snapshot, opponentColor);
    
    for (const auto& move : opponentMoves) {
        if (move.to == targetPosition) {
            for (const auto& pieceSnap : snapshot.pieces) {
                if (pieceSnap.logicalPosition == move.from && pieceSnap.color == opponentColor && pieceSnap.state != PieceState::Captured) {
                    bool alreadyAdded = false;
                    for (const auto& existing : threateningPieces) {
                        if (existing.logicalPosition == pieceSnap.logicalPosition) {
                            alreadyAdded = true;
                            break;
                        }
                    }
                    if (!alreadyAdded) {
                        threateningPieces.push_back(pieceSnap);
                    }
                }
            }
        }
    }
    return threateningPieces;
}

bool ThreatAnalyzer::isThreatened(
    const view::GameSnapshot& snapshot,
    const Position& targetPosition,
    PlayerColor defenderColor) const {
    
    PlayerColor opponentColor = (defenderColor == PlayerColor::White) ? PlayerColor::Black : PlayerColor::White;
    MoveGenerator generator;
    auto opponentMoves = generator.generateForPlayer(snapshot, opponentColor);
    
    for (const auto& move : opponentMoves) {
        if (move.to == targetPosition) {
            return true;
        }
    }
    return false;
}

} // namespace kungfu