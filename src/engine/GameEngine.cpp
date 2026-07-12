#include "engine/GameEngine.hpp"
#include "common/GameConfig.hpp"
#include <algorithm>
#include <cmath>

namespace kungfu {

// הבנאי המעודכן המקבל קונפיגורציית משחק דינמית
GameEngine::GameEngine(std::shared_ptr<IBoard> board, 
                       std::shared_ptr<RuleEngine> ruleEngine,
                       GameConfig config) noexcept
    : board_(std::move(board))
    , ruleEngine_(std::move(ruleEngine))
    , arbiter_(board_)
    , config_(std::move(config)) {}

MoveResult GameEngine::requestMove(const Position& from, const Position& to) {
    if (gameOver_) {
        return {false, "game_over"};
    }

    // שימוש בקונפיגורציה דינמית של תנועה סימולטנית
    if (!config_.allowSimultaneousMovement && arbiter_.hasActiveMotion()) {
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

    // בדיקה האם הכלי עסוק (בתנועה, באוויר או בצינון)
    bool isPieceBusy = arbiter_.isPieceMoving(piece) || 
                       piece->state() == PieceState::Airborne || 
                       arbiter_.isOnCooldown(piece, currentTimeMs_);
    
    if (isPieceBusy) {
        return handlePremoveRegistration(piece, from, to);
    }

    // זיהוי בקשת קפיצה (Jump Request - תנועה מאותה משבצת לעצמה)
    if (from == to) {
        return handleJumpRequest(piece, from);
    }

    // טיפול במהלכי תנועה רגילים
    return handleStandardMove(piece, from, to);
}

MoveResult GameEngine::handlePremoveRegistration(const PiecePtr& piece, const Position& from, const Position& to) noexcept {
    // שימוש בקונפיגורציה דינמית לזיהוי האם מהלכים מראש מאופשרים
    if (config_.enablePremoves) {
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

MoveResult GameEngine::handleJumpRequest(const PiecePtr& piece, const Position& pos) noexcept {
    if (piece->state() == PieceState::Captured) {
        return {false, "captured_piece_cannot_jump"};
    }

    piece->setState(PieceState::Airborne);
    // שימוש במשך קפיצה דינמי
    arbiter_.startMotion(piece, pos, pos, currentTimeMs_, config_.jumpDurationMs);

    return {true, "jump_started"};
}

MoveResult GameEngine::handleStandardMove(const PiecePtr& piece, const Position& from, const Position& to) noexcept {
    auto validation = ruleEngine_->validateMove(from, to);
    if (!validation.isValid) {
        return {false, validation.reason};
    }

    int dr = std::abs(to.row() - from.row());
    int dc = std::abs(to.col() - from.col());
    int distance = std::max(dr, dc);
    // שימוש במהירות תנועה דינמית למשבצת
    int durationMs = distance * config_.msPerCellSpeed;

    piece->setState(PieceState::Moving);
    arbiter_.startMotion(piece, from, to, currentTimeMs_, durationMs);

    return {true, "ok"};
}

void GameEngine::wait(int ms) noexcept {
    if (ms <= 0 || !board_) {
        return;
    }

    // הגדרת חוק ההכתרה כלמדא מקומית (חוקי שחמט בסיסיים)
    auto chessPromotionRule = [this](const PiecePtr& piece, const Position& to) -> PiecePtr {
        if (piece->type() == PieceType::Pawn) {
            // זיהוי הגעה לשורה האחרונה בהתאם לצבע הכלי
            bool shouldPromote = (piece->color() == PlayerColor::White && to.row() == board_->rows() - 1) ||
                                 (piece->color() == PlayerColor::Black && to.row() == 0);
            if (shouldPromote) {
                auto queen = std::make_shared<Piece>(PieceType::Queen, piece->color(), to);
                board_->replacePiece(to, queen); // החלפה לוגית בלוח
                return queen;
            }
        }
        return piece;
    };

    // הרצת השעון ב-Arbiter עם פונקציית ה-Callback לחוק ההכתרה
    auto events = arbiter_.advanceTime(ms, currentTimeMs_, chessPromotionRule);
    for (const auto& event : events) {
        if (event.capturedKing) {
            gameOver_ = true;
        }
    }

    // לאחר קידום הזמן, ננסה להפעיל Premoves שהפכו לחוקיים ופנויים כעת
    if (config_.enablePremoves && !gameOver_) {
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
            requestMove(data.from, data.to);

            // בכל מקרה מוחקים את ה-Premove מהתור
            it = premoves_.erase(it);
            continue;
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

}  // namespace kungfu