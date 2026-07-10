#include "realtime/RealTimeArbiter.hpp"
#include "common/GameConfig.hpp"
#include <algorithm>

namespace kungfu {

RealTimeArbiter::RealTimeArbiter(std::shared_ptr<IBoard> board) noexcept
    : board_(std::move(board)) {}

bool RealTimeArbiter::hasActiveMotion() const noexcept {
    return !activeMotions_.empty();
}

void RealTimeArbiter::startMotion(PiecePtr piece, const Position& from, const Position& to, int currentTimeMs, int durationMs) noexcept {
    activeMotions_.emplace_back(std::move(piece), from, to, currentTimeMs, durationMs);
}

std::vector<ArrivalEvent> RealTimeArbiter::advanceTime(int ms, int& currentTimeMs) noexcept {
    std::vector<ArrivalEvent> events;
    currentTimeMs += ms;

    // --- לוגיקת התנגשות עוינות בדרך (Enemy Collisions Mid-route) ---
    // אם מופעלת תנועה סימולטנית, נבדוק האם שני כלים עוינים מנסים להחליף מקומות
    if (GameConfig::kAllowSimultaneousMovement && activeMotions_.size() >= 2) {
        for (size_t i = 0; i < activeMotions_.size(); ++i) {
            for (size_t j = i + 1; j < activeMotions_.size(); ++j) {
                auto& m1 = activeMotions_[i];
                auto& m2 = activeMotions_[j];

                // בדיקה שהם אויבים ושהם מחליפים מקומות (מצטלבים על אותו קו באותו סגמנט)
                if (m1.piece()->color() != m2.piece()->color()) {
                    if (m1.from() == m2.to() && m1.to() == m2.from()) {
                        // זיהוי התנגשות! הכלל: מי שיצא קודם (startTime קטן יותר) מנצח
                        auto& winner = (m1.startTime() <= m2.startTime()) ? m1 : m2;
                        auto& loser = (m1.startTime() <= m2.startTime()) ? m2 : m1;

                        // חיסול המפסיד: הפיכתו ל-Captured והסרתו מהלוח
                        loser.piece()->setState(PieceState::Captured);
                        cooldowns_.erase(loser.piece().get());
                        board_->removePiece(loser.from()); // נמחק ממשבצת המקור שלו כי הוא מת בדרך

                        events.push_back({loser.from(), loser.from(), loser.piece(), false, true}); // אירוע ביטול/חיסול
                    }
                }
            }
        }

        // הסרת התנועה של הכלים שחוסלו מרשימת התנועות הפעילות
        activeMotions_.erase(
            std::remove_if(activeMotions_.begin(), activeMotions_.end(),
                [](const Motion& m) { return m.piece()->state() == PieceState::Captured; }),
            activeMotions_.end()
        );
    }

    // --- תהליך הגעה ליעד הרגיל ---
     for (auto it = activeMotions_.begin(); it != activeMotions_.end(); ) {
        if (currentTimeMs >= it->arrivalTime()) {
            Position from = it->from();
            Position to = it->to();
            auto piece = it->piece();

            // 1. טיפול בסיום קפיצה (Landing Normally)
            if (from == to && piece->state() == PieceState::Airborne) {
                piece->setState(PieceState::Idle); // נחיתה נורמלית
                cooldowns_[piece.get()] = currentTimeMs + GameConfig::kCooldownDurationMs; // רישום צינון
                
                events.push_back({from, to, piece, false, false});
                it = activeMotions_.erase(it);
                continue;
            }

            // 2. הגעה רגילה של כלי נע: בדיקה מול כלי קופץ (Airborne) ביעד
            auto targetPieceOpt = board_->pieceAt(to);

            if (targetPieceOpt.has_value() && targetPieceOpt.value() && targetPieceOpt.value()->state() == PieceState::Airborne) {
                auto airbornePiece = targetPieceOpt.value();
                
                if (airbornePiece->color() != piece->color()) {
                    // א. הגעת אויב: הכלי הקופץ אוכל אותו! 
                    // האויב המגיע מבוטל ומוסר מהמשחק, הכלי הקופץ לא זז ונשאר במשבצת שלו.
                    piece->setState(PieceState::Captured);
                    cooldowns_.erase(piece.get());
                    board_->removePiece(from); // הסרת האויב ממשבצת המוצא שלו (כי הוא מעולם לא נחת ביעד)

                    events.push_back({from, from, piece, false, true}); // אירוע ביטול/אכילה של האויב
                    it = activeMotions_.erase(it);
                    continue;
                } else {
                    // ב. הגעת ידידותי: התנגשות ידידותית! מהלך הכלי שהגיע מבוטל והוא חוזר למוצא שלו.
                    piece->setState(PieceState::Idle);
                    events.push_back({from, from, piece, false, true});
                    it = activeMotions_.erase(it);
                    continue;
                }
            }

            // 3. הגעה רגילה ללא כלים קופצים (ללא שינוי...)
            if (targetPieceOpt.has_value() && targetPieceOpt.value() && targetPieceOpt.value()->color() == piece->color()) {
                piece->setState(PieceState::Idle);
                events.push_back({from, from, piece, false, true});
                it = activeMotions_.erase(it);
                continue;
            }

            bool capturedKing = false;
            if (targetPieceOpt.has_value() && targetPieceOpt.value()) {
                auto targetPiece = targetPieceOpt.value();
                if (targetPiece->type() == PieceType::King) {
                    capturedKing = true;
                }
                targetPiece->setState(PieceState::Captured);
                cooldowns_.erase(targetPiece.get());
                board_->removePiece(to);
            }

            board_->movePiece(from, to);
            piece->setState(PieceState::Idle);

            cooldowns_[piece.get()] = currentTimeMs + GameConfig::kCooldownDurationMs;

            events.push_back({from, to, piece, capturedKing, false});
            it = activeMotions_.erase(it);
        } else {
            ++it;
        }
    }

    return events;
}


bool RealTimeArbiter::isOnCooldown(const PiecePtr& piece, int currentTimeMs) const noexcept {
    if (!piece) return false;
    auto it = cooldowns_.find(piece.get());
    if (it != cooldowns_.end()) {
        return currentTimeMs < it->second;
    }
    return false;
}

bool RealTimeArbiter::isPieceMoving(const PiecePtr& piece) const noexcept {
    if (!piece) return false;
    for (const auto& motion : activeMotions_) {
        if (motion.piece() == piece) {
            return true;
        }
    }
    return false;
}

std::optional<Motion> RealTimeArbiter::getMotionForPiece(const PiecePtr& piece) const noexcept {
    if (!piece) {
        return std::nullopt;
    }
    for (const auto& motion : activeMotions_) {
        if (motion.piece() == piece) {
            return motion;
        }
    }
    return std::nullopt;
}

}  // namespace kungfu