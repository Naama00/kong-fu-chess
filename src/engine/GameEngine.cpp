#include "engine/GameEngine.hpp"
#include "engine/GameSnapshot.hpp"
#include "common/GameConfig.hpp"
#include <algorithm>
#include <cmath>

namespace kungfu {

GameEngine::GameEngine(std::shared_ptr<IBoard> board, std::shared_ptr<RuleEngine> ruleEngine) noexcept
    : board_(std::move(board))
    , ruleEngine_(std::move(ruleEngine))
    , arbiter_(board_) {}

MoveResult GameEngine::requestMove(const Position& from, const Position& to) {
    if (gameOver_) {
        return {false, "game_over"};
    }

    if (!GameConfig::kAllowSimultaneousMovement && arbiter_.hasActiveMotion()) {
        return {false, "motion_in_progress"};
    }

    if (!board_ || !ruleEngine_) {
        return {false, "internal_error"};
    }

    auto sourcePieceOpt = board_->pieceAt(from);
    if (!sourcePieceOpt.has_value() || !sourcePieceOpt.value()) {
        return {false, "empty_source"};
    }
    auto piece = sourcePieceOpt.value();

    // בדיקה אם הכלי עסוק (בתנועה, באוויר או בצינון)
    bool isPieceBusy = arbiter_.isPieceMoving(piece) || 
                       piece->state() == PieceState::Airborne || 
                       arbiter_.isOnCooldown(piece, currentTimeMs_);
    
    if (isPieceBusy) {
        if (GameConfig::kEnablePremoves) {
            auto it = std::find_if(premoves_.begin(), premoves_.end(), [&](const auto& pair) {
                return pair.first == piece;
            });
            if (it != premoves_.end()) {
                it->second = {from, to};
            } else {
                premoves_.push_back({piece, {from, to}});
            }
            return {true, "premove_registered"};
        } else {
            if (arbiter_.isPieceMoving(piece) || piece->state() == PieceState::Airborne) {
                return {false, "motion_in_progress"};
            }
            return {false, "piece_on_cooldown"};
        }
    }

    // --- זיהוי בקשת קפיצה (Jump Request) ---
    if (from == to) {
        // מניעת קפיצה של כלי מבוטל (לשלמות, למרות ש-isPieceBusy כבר חוסם כלים לא פנויים)
        if (piece->state() == PieceState::Captured) {
            return {false, "captured_piece_cannot_jump"};
        }

        // קפיצה נמשכת בדיוק 1,000 מילישניות
        int jumpDurationMs = 1000;
        piece->setState(PieceState::Airborne); // שינוי המצב ל-Airborne
        arbiter_.startMotion(piece, from, to, currentTimeMs_, jumpDurationMs);

        return {true, "jump_started"};
    }

    // מהלך רגיל (from != to) - ללא שינוי...
    auto validation = ruleEngine_->validateMove(from, to);
    if (!validation.isValid) {
        return {false, validation.reason};
    }

    int dr = std::abs(to.row() - from.row());
    int dc = std::abs(to.col() - from.col());
    int distance = std::max(dr, dc);
    int durationMs = distance * GameConfig::kMsPerCellSpeed;

    piece->setState(PieceState::Moving);
    arbiter_.startMotion(piece, from, to, currentTimeMs_, durationMs);

    return {true, "ok"};
}

void GameEngine::wait(int ms) noexcept {
    if (ms <= 0 || !board_) {
        return;
    }

    auto events = arbiter_.advanceTime(ms, currentTimeMs_);
    for (const auto& event : events) {
        if (event.capturedKing) {
            gameOver_ = true;
        }
    }

    // לאחר קידום הזמן, ננסה להפעיל Premoves שהפכו לחוקיים ופנויים כעת
    if (GameConfig::kEnablePremoves && !gameOver_) {
        processPremoves();
    }
}

void GameEngine::processPremoves() noexcept {
    for (auto it = premoves_.begin(); it != premoves_.end(); ) {
        auto piece = it->first;
        auto data = it->second;

        // בדיקה שהכלי עדיין על הלוח ולא הושמד בזמן הצינון/תנועה
        if (piece->state() == PieceState::Captured) {
            it = premoves_.erase(it);
            continue;
        }

        // בדיקה שהכלי התפנה ויכול לזוז
        bool isPieceBusy = arbiter_.isPieceMoving(piece) || arbiter_.isOnCooldown(piece, currentTimeMs_);
        if (!isPieceBusy) {
            // שליחת בקשת תנועה רגילה
            auto result = requestMove(data.from, data.to);
            if (result.isAccepted && result.reason == "ok") {
                // הצליח לצאת לדרך! נמחק מהתור
                it = premoves_.erase(it);
                continue;
            } else {
                // המהלך הפך ללא חוקי כעת (למשל, חסימה חדשה) - נבטל את ה-Premove
                it = premoves_.erase(it);
                continue;
            }
        }
        ++it;
    }
}

std::optional<PlayerColor> GameEngine::getPieceColorAt(const Position& pos) const {
    if (!board_) {
        return std::nullopt;
    }
    auto pieceOpt = board_->pieceAt(pos);
    if (pieceOpt.has_value() && pieceOpt.value()) {
        return pieceOpt.value()->color();
    }
    return std::nullopt;
}

bool GameEngine::hasPieceAt(const Position& pos) const {
    if (!board_) {
        return false;
    }
    auto pieceOpt = board_->pieceAt(pos);
    return pieceOpt.has_value() && pieceOpt.value() != nullptr;
}

int GameEngine::getBoardRows() const {
    return board_ ? board_->rows() : 0;
}

int GameEngine::getBoardCols() const {
    return board_ ? board_->cols() : 0;
}

void GameEngine::wait(int ms) noexcept {
    if (ms <= 0 || !board_) {
        return;
    }

    auto events = arbiter_.advanceTime(ms, currentTimeMs_);
    for (const auto& event : events) {
        if (event.capturedKing) {
            gameOver_ = true;
        }
    }
}

GameSnapshot GameEngine::getSnapshot(std::optional<Position> selectedCell) const noexcept {
    GameSnapshot snap;
    snap.boardCols = getBoardCols();
    snap.boardRows = getBoardRows();
    snap.isGameOver = gameOver_;
    snap.selectedCell = selectedCell;

    if (!board_) {
        return snap;
    }

    // הגדרת משתנה עזר מקומי למניעת הכפלות בקוד בקבוע קשיח
    const float cSize = static_cast<float>(GameConfig::kDefaultCellSize);

    for (const auto& piece : board_->pieces()) {
        if (!piece || piece->state() == PieceState::Captured) {
            continue;
        }

        PieceSnapshot pSnap;
        pSnap.type = piece->type();
        pSnap.color = piece->color();
        pSnap.logicalPosition = piece->position();
        pSnap.state = piece->state();

        // שינוי 2: שימוש ב-cSize במקום ב-100.0f
        float currentX = piece->position().col() * cSize;
        float currentY = piece->position().row() * cSize;

        pSnap.pixelX = currentX;
        pSnap.pixelY = currentY;

        if (piece->state() == PieceState::Moving) {
            auto motionOpt = arbiter_.getMotionForPiece(piece);
            if (motionOpt.has_value()) {
                const auto& motion = motionOpt.value();
                // שינוי 3: שימוש ב-cSize למציאת קואורדינטות הפיקסלים בתחילת וסוף המהלך
                float startX = motion.from().col() * cSize;
                float startY = motion.from().row() * cSize;
                float endX = motion.to().col() * cSize;
                float endY = motion.to().row() * cSize;

                int duration = motion.arrivalTime() - motion.startTime();
                if (duration > 0) {
                    int elapsed = currentTimeMs_ - motion.startTime();
                    float t = static_cast<float>(elapsed) / static_cast<float>(duration);

                    t = std::max(0.0f, std::min(t, 1.0f));

                    pSnap.pixelX = startX + t * (endX - startX);
                    pSnap.pixelY = startY + t * (endY - startY);
                }
            }
        }

        snap.pieces.push_back(pSnap);
    }

    return snap;
}

}  // namespace kungfu