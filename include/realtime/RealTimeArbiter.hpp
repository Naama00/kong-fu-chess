#pragma once

#include <memory>
#include <optional>
#include <vector>
#include <unordered_map>
#include <functional> 
#include "realtime/Motion.hpp"
#include "board/IBoard.hpp"

namespace kungfu {

struct ArrivalEvent {
    Position from;
    Position to;
    PiecePtr piece;
    bool capturedKing;
    bool cancelled; // דגל המציין האם התנועה בוטלה עקב חסימה
};

// חתימת פונקציה חיצונית לניהול חוקי הכתרה או שינוי של הכלי בנחיתה
using PromotionHandler = std::function<PiecePtr(const PiecePtr&, const Position&)>;

class RealTimeArbiter {
public:
    explicit RealTimeArbiter(std::shared_ptr<IBoard> board, GameConfig config = GameConfig{}) noexcept;

    bool hasActiveMotion() const noexcept;
    void startMotion(PiecePtr piece, const Position& from, const Position& to, int currentTimeMs, int durationMs) noexcept;
    
    // עדכון: המתודה מקבלת כעת Callback אופציונלי לניהול חוק ההכתרה מחוץ למחלקה
    std::vector<ArrivalEvent> advanceTime(int ms, int& currentTimeMs, PromotionHandler promoteCallback = nullptr) noexcept;

    bool isOnCooldown(const PiecePtr& piece, int currentTimeMs) const noexcept;
    bool isPieceMoving(const PiecePtr& piece) const noexcept;
    std::optional<Motion> getMotionForPiece(const PiecePtr& piece) const noexcept;

private:
    std::shared_ptr<IBoard> board_;
    std::vector<Motion> activeMotions_; 
    std::unordered_map<const Piece*, int> cooldowns_;
    GameConfig config_;

    void handleMidRouteCollisions(std::vector<ArrivalEvent>& events) noexcept;
    
    // עדכון: פונקציית הנחיתה מקבלת את ה-Callback
    bool processSingleArrival(
        std::vector<Motion>::iterator it, 
        int currentTimeMs, 
        std::vector<ArrivalEvent>& events,
        const PromotionHandler& promoteCallback
    ) noexcept;
};

}  // namespace kungfu