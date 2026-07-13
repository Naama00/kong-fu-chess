#include "rules/CollisionResolver.hpp"
#include <cmath>
#include <algorithm>

namespace kungfu {

// פונקציית העזר הקיימת שלך, נשתמש בה כדי למצוא משבצת פנויה אחרונה
Position findLastVacantPositionOnPath(
    const Position& from,
    const Position& to,
    const std::shared_ptr<IBoard>& board
) noexcept {
    int r1 = from.row(); int c1 = from.col();
    int r2 = to.row(); int c2 = to.col();
    int dr = r2 - r1; int dc = c2 - c1;

    if (dr == 0 && dc == 0) {
        if (!board->pieceAt(from).has_value()) return from;
    } else {
        int stepR = (dr > 0) ? 1 : ((dr < 0) ? -1 : 0);
        int stepC = (dc > 0) ? 1 : ((dc < 0) ? -1 : 0);

        std::vector<Position> path;
        int curR = r2 - stepR;
        int curC = c2 - stepC;

        while (curR != r1 || curC != c1) {
            path.emplace_back(curR, curC);
            curR -= stepR;
            curC -= stepC;
        }
        path.push_back(from);

        for (const auto& pos : path) {
            if (!board->pieceAt(pos).has_value()) {
                return pos;
            }
        }
    }
    return from;
}

void CollisionResolver::resolveMidRouteCollision(
    const Motion& firstArrived, // הכלי שהגיע ראשון למשבצת ההתנגשות (או מותקף)
    const Motion& secondArrived, // הכלי שהגיע שני (התוקף / החוסם)
    std::vector<ArrivalEvent>& events
) noexcept {
    
    // מקרה 1: כלי אויב - כלי אויב הגיע למשבצת שבה כלי אחר נמצא/עובר -> אכילה!
    if (firstArrived.piece()->color() != secondArrived.piece()->color()) {
        // הכלי שהגיע שני (התוקף) אוכל את הכלי שהיה שם קודם
        firstArrived.piece()->setState(PieceState::Captured);
        cooldownTracker_.clear(firstArrived.piece()->id());
        
        firstArrived.piece()->setPosition(firstArrived.from());
        board_->removePiece(firstArrived.from());

        events.push_back({firstArrived.from(), firstArrived.from(), firstArrived.piece(), false, true});
        
        // הערה: הכלי השני (התוקף) ימשיך ליעדו כרגיל או ייעצר במשבצת האכילה, תלוי בלוגיקת ה-Arbiter שלכם. 
        // מומלץ לעדכן את היעד של secondArrived לנקודת המפגש אם רוצים שהוא ייעצר שם.
    } 
    // מקרה 2: כלים ידידותיים - הכלי שהגיע מאוחר יותר נעצר במשבצת האחרונה הפנויה
    else {
        // נמצא את המשבצת האחרונה הפנויה במסלול של הכלי השני (המאוחר)
        Position stopPos = findLastVacantPositionOnPath(secondArrived.from(), secondArrived.to(), board_);
        
        secondArrived.piece()->setState(PieceState::Idle);
        secondArrived.piece()->setPosition(stopPos);
        
        cooldownTracker_.setCooldown(secondArrived.piece()->id(), board_->rows() /* או currentTime שהיה מועבר */ + config_.cooldownDurationMs);
        events.push_back({secondArrived.from(), stopPos, secondArrived.piece(), false, true});
        
        // אנו מסמנים למערכת שהתנועה של secondArrived הסתיימה מוקדם מהצפוי
    }
}

bool CollisionResolver::resolveArrival(
    const Motion& motion,
    int currentTimeMs,
    std::vector<ArrivalEvent>& events,
    const PromotionHandler& promoteCallback
) noexcept {
    Position from = motion.from();
    Position to = motion.to();
    auto piece = motion.piece();

    // מאפשרים החזרה מוקדמת רק אם משבצת הנחיתה בקפיצה במקום אכן פנויה
    if (from == to && !board_->pieceAt(to).has_value()) {
        piece->setState(PieceState::Idle);
        piece->setPosition(to); // החזרת המיקום הלוגי למקום הנחיתה
        cooldownTracker_.setCooldown(piece->id(), currentTimeMs + config_.cooldownDurationMs);
        events.push_back({from, to, piece, false, false});
        return true;
    }

    auto targetPieceOpt = board_->pieceAt(to);

    // בדיקת חסימה על ידי כלי ידידותי ביעד
    bool isFriendlyBlock = false;
    if (targetPieceOpt.has_value() && targetPieceOpt.value()) {
        auto targetPiece = targetPieceOpt.value();
        if (targetPiece->color() == piece->color()) {
            if (piece->type() != PieceType::Knight) { // פרשים מתעלמים מחסימות ידידותיות (מכים אותן)
                isFriendlyBlock = true;
            }
        }
    }

    Position finalDestination = to;

    if (isFriendlyBlock) {
        // מציאת המשבצת הפנויה האחרונה לאורך מסלול הנסיגה
        finalDestination = findLastVacantPositionOnPath(from, to, board_);

        piece->setState(PieceState::Idle);
        piece->setPosition(finalDestination); // מיקום הכלי במשבצת הנסיגה
        
        // הטלת צינון כי הכלי בכל זאת ביצע תנועה והתעייף
        cooldownTracker_.setCooldown(piece->id(), currentTimeMs + config_.cooldownDurationMs);
        
        events.push_back({from, finalDestination, piece, false, true});
        return true;
    }

    // נחיתה רגילה ללא חסימות ידידותיות
    // אכילת כלי עוין ביעד (אם קיים)
    bool capturedKing = false;
    if (targetPieceOpt.has_value() && targetPieceOpt.value()) {
        auto targetPiece = targetPieceOpt.value();
        if (targetPiece->type() == PieceType::King) {
            capturedKing = true;
        }
        targetPiece->setState(PieceState::Captured);
        cooldownTracker_.clear(targetPiece->id());
        board_->removePiece(targetPiece->position());
    }

    piece->setPosition(finalDestination);
    piece->setState(PieceState::Idle);
    piece->markMoved();

    PiecePtr finalPiece = piece;
    if (promoteCallback) {
        finalPiece = promoteCallback(piece, finalDestination);
    }
    if (finalPiece != piece) {
        cooldownTracker_.clear(piece->id());
    }
    cooldownTracker_.setCooldown(finalPiece->id(), currentTimeMs + config_.cooldownDurationMs);
    events.push_back({from, finalDestination, finalPiece, capturedKing, false});

    return true;
}

} // namespace kungfu