#pragma once

#include "rules/IPieceRule.hpp"

namespace kungfu {

class RookRule : public IPieceRule {
public:
    std::vector<Position> getLegalDestinations(const IBoard& board, const Piece& piece) const override;
};

class BishopRule : public IPieceRule {
public:
    std::vector<Position> getLegalDestinations(const IBoard& board, const Piece& piece) const override;
};

class QueenRule : public IPieceRule {
public:
    std::vector<Position> getLegalDestinations(const IBoard& board, const Piece& piece) const override;
};

class KnightRule : public IPieceRule {
public:
    std::vector<Position> getLegalDestinations(const IBoard& board, const Piece& piece) const override;
};

class KingRule : public IPieceRule {
public:
    std::vector<Position> getLegalDestinations(const IBoard& board, const Piece& piece) const override;
};

class PawnRule : public IPieceRule {
public:
    std::vector<Position> getLegalDestinations(const IBoard& board, const Piece& piece) const override;
};

class PieceRuleFactory {
public:
    // מחזיר הפנייה סינגלטונית לחוק המתאים לסוג הכלי, ללא הקצאת זיכרון דינמית
    static const IPieceRule& getRule(PieceType type) noexcept;
};

}  // namespace kungfu