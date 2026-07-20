#pragma once

#include "engine/rules/IPromotionRule.hpp"

namespace kungfu {

// Classic crowning rule: A pawn that reaches the last rank automatically becomes a queen.
class ChessPromotionRule : public IPromotionRule {
public:
    PiecePtr maybePromote(const PiecePtr& piece, const Position& to, IBoard& board) const override;
};

}  // namespace kungfu