// engine/analysis/ThreatAnalyzer.hpp
#pragma once

#include "engine/analysis/IThreatAnalyzer.hpp"
#include "engine/analysis/MoveGenerator.hpp"

namespace kungfu {

class ThreatAnalyzer : public IThreatAnalyzer {
public:
    ThreatAnalyzer() = default;
    ~ThreatAnalyzer() override = default;

    std::vector<view::PieceSnapshot> getThreateningPieces(
        const view::GameSnapshot& snapshot,
        const Position& targetPosition,
        PlayerColor defenderColor) const override;

    bool isThreatened(
        const view::GameSnapshot& snapshot,
        const Position& targetPosition,
        PlayerColor defenderColor) const override;
};

} // namespace kungfu