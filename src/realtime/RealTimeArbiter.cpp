#include "realtime/RealTimeArbiter.hpp"
#include "common/GameConfig.hpp"
#include <algorithm>

namespace kungfu {

RealTimeArbiter::RealTimeArbiter(std::shared_ptr<IBoard> board, GameConfig config) noexcept
    : board_(std::move(board)), config_(std::move(config)) {}

bool RealTimeArbiter::hasActiveMotion() const noexcept {
    return !activeMotions_.empty();
}

void RealTimeArbiter::startMotion(PiecePtr piece, const Position& from, const Position& to, int currentTimeMs, int durationMs) noexcept {
    activeMotions_.emplace_back(std::move(piece), from, to, currentTimeMs, durationMs);
}

// עדכון: קבלת ה-Callback והעברתו לניהול הנחיתות
std::vector<ArrivalEvent> RealTimeArbiter::advanceTime(int ms, int& currentTimeMs, PromotionHandler promoteCallback) noexcept {
    std::vector<ArrivalEvent> events;
    currentTimeMs += ms;

    // 1. טיפול בהתנגשויות תוך כדי תנועה (Mid-route Collisions)
    handleMidRouteCollisions(events);

    // 2. עיבוד הגעה ליעדים
    for (auto it = activeMotions_.begin(); it != activeMotions_.end(); ) {
        if (currentTimeMs >= it->arrivalTime()) {
            if (processSingleArrival(it, currentTimeMs, events, promoteCallback)) {
                it = activeMotions_.erase(it);
            } else {
                ++it;
            }
        } else {
            ++it;
        }
    }

    return events;
}

void RealTimeArbiter::handleMidRouteCollisions(std::vector<ArrivalEvent>& events) noexcept {
    if (!config_.allowSimultaneousMovement || activeMotions_.size() < 2) {
        return;
    }

    for (size_t i = 0; i < activeMotions_.size(); ++i) {
        for (size_t j = i + 1; j < activeMotions_.size(); ++j) {
            auto& m1 = activeMotions_[i];
            auto& m2 = activeMotions_[j];

            // בדיקה האם מדובר בכלים עוינים שמנסים להחליף מקומות
            if (m1.piece()->color() != m2.piece()->color()) {
                if (m1.from() == m2.to() && m1.to() == m2.from()) {
                    // כלי שהתחיל לנוע קודם מנצח בהתנגשות
                    auto& winner = (m1.startTime() <= m2.startTime()) ? m1 : m2;
                    auto& loser = (m1.startTime() <= m2.startTime()) ? m2 : m1;

                    loser.piece()->setState(PieceState::Captured);
                    cooldowns_.erase(loser.piece()->id());     
                    board_->removePiece(loser.from());

                    events.push_back({loser.from(), loser.from(), loser.piece(), false, true});
                }
            }
        }
    }

    // ניקוי הכלים שחוסלו מרשימת התנועות הפעילות
    activeMotions_.erase(
        std::remove_if(activeMotions_.begin(), activeMotions_.end(),
            [](const Motion& m) { return m.piece()->state() == PieceState::Captured; }),
        activeMotions_.end()
    );
}

bool RealTimeArbiter::processSingleArrival(
    std::vector<Motion>::iterator it, 
    int currentTimeMs, 
    std::vector<ArrivalEvent>& events,
    const PromotionHandler& promoteCallback
) noexcept {
    Position from = it->from();
    Position to = it->to();
    auto piece = it->piece();

    // 1. טיפול בנחיתה תקינה של כלי קופץ (Airborne)
    if (from == to && piece->state() == PieceState::Airborne) {
        piece->setState(PieceState::Idle);
        cooldowns_[piece->id()] = currentTimeMs + config_.cooldownDurationMs; 
        events.push_back({from, to, piece, false, false});
        return true;
    }

    auto targetPieceOpt = board_->pieceAt(to);

    // 2. הגעה למשבצת שבה יש כלי קופץ (Airborne)
    if (targetPieceOpt.has_value() && targetPieceOpt.value() && targetPieceOpt.value()->state() == PieceState::Airborne) {
        auto airbornePiece = targetPieceOpt.value();
        
        if (airbornePiece->color() != piece->color()) {
            piece->setState(PieceState::Captured);
            cooldowns_.erase(piece->id()); 
            board_->removePiece(from);

            events.push_back({from, from, piece, false, true});
            return true;
        } else {
            piece->setState(PieceState::Idle);
            events.push_back({from, from, piece, false, true});
            return true;
        }
    }

    // 3. הגעה רגילה למשבצת עם כלי ידידותי (חסימה)
    if (targetPieceOpt.has_value() && targetPieceOpt.value() && targetPieceOpt.value()->color() == piece->color()) {
        piece->setState(PieceState::Idle);
        events.push_back({from, from, piece, false, true});
        return true;
    }

    // 4. אכילה רגילה / הגעה חלקה ליעד ריק
    bool capturedKing = false;
    if (targetPieceOpt.has_value() && targetPieceOpt.value()) {
        auto targetPiece = targetPieceOpt.value();
        if (targetPiece->type() == PieceType::King) {
            capturedKing = true;
        }
        targetPiece->setState(PieceState::Captured);
        cooldowns_.erase(targetPiece->id());  
        board_->removePiece(to);
    }

    board_->movePiece(from, to);
    piece->setState(PieceState::Idle);
    piece->markMoved();

    PiecePtr finalPiece = piece;
    if (promoteCallback) {
        finalPiece = promoteCallback(piece, to);
    }
    // ניקוי רשומת הצינון של הכלי המקורי אם הוחלף (למשל: רגלי שהוכתר),
    // כדי למנוע הצטברות רשומות יתומות ב-cooldowns_ שלעולם לא יישאלו יותר
    if (finalPiece != piece) {
    cooldowns_.erase(piece->id()); 
    }
    cooldowns_[finalPiece->id()] = currentTimeMs + config_.cooldownDurationMs;   
    events.push_back({from, to, finalPiece, capturedKing, false});

    return true;
}

bool RealTimeArbiter::isOnCooldown(const PiecePtr& piece, int currentTimeMs) const noexcept {
    if (!piece) return false;
    auto it = cooldowns_.find(piece->id());  
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