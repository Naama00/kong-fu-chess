#include "rules/RuleEngine.hpp"
#include "common/GameConfig.hpp"
#include <cmath>

namespace kungfu {

RuleEngine::RuleEngine(BoardPtr board) : board_(std::move(board)) {}

bool RuleEngine::isValidMove(const Position& from, const Position& to) const {
    if (!board_) {
        return false;
    }

    const auto sourcePiece = board_->pieceAt(from);
    if (!sourcePiece.has_value() || !sourcePiece.value() || !sourcePiece.value()->isMovable()) {
        return false;
    }

    // --- חוקי תנועה מיוחדים עבור רגלי (Pawn) ---
    if (sourcePiece.value()->type() == PieceType::Pawn) {
        const auto targetPiece = board_->pieceAt(to);
        const int rowDelta = to.row() - from.row();
        const int colDelta = std::abs(to.col() - from.col());
        const auto color = sourcePiece.value()->color();

        if (color == PlayerColor::White) {
            // תנועה ישרה קדימה - אסור לאכול קדימה!
            if (colDelta == 0) {
                if (targetPiece.has_value()) {
                    return false; // הדרך חסומה, רגלי לא יכול לנוע או לאכול ישר קדימה
                }
                const bool oneStep = (rowDelta == 1);
                const bool twoStep = (from.row() == GameConfig::kWhitePawnStartRow && rowDelta == 2);
                return oneStep || twoStep;
            }
            // אכילה באלכסון - חייב להיות כלי עוין ביעד כדי לאפשר את המהלך
            if (colDelta == 1 && rowDelta == 1) {
                return targetPiece.has_value() && targetPiece.value()->color() != color;
            }
        } else {
            // תנועה ישרה קדימה - אסור לאכול קדימה!
            if (colDelta == 0) {
                if (targetPiece.has_value()) {
                    return false; // הדרך חסומה, רגלי לא יכול לנוע או לאכול ישר קדימה
                }
                const bool oneStep = (rowDelta == -1);
                const bool twoStep = (from.row() == GameConfig::kBlackPawnStartRow && rowDelta == -2);
                return oneStep || twoStep;
            }
            // אכילה באלכסון - חייב להיות כלי עוין ביעד כדי לאפשר את המהלך
            if (colDelta == 1 && rowDelta == -1) {
                return targetPiece.has_value() && targetPiece.value()->color() != color;
            }
        }
        return false; // כל תנועה אחרת של רגלי אינה חוקית
    }

    // עבור שאר הכלים (מלך, מלכה, צריח, רץ ופרש) - נשענים על הגיאומטריה הרגילה
    return movementSystem_.isValidMove(*sourcePiece.value(), from, to);
}

bool RuleEngine::isPawnPromotion(const Position& to, PlayerColor color) const {
    return color == PlayerColor::White
               ? to.row() == GameConfig::kWhitePawnPromotionRow
               : to.row() == GameConfig::kBlackPawnPromotionRow;
}

}  // namespace kungfu