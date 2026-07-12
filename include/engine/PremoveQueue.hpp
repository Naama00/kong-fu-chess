#pragma once

#include <vector>
#include <functional>
#include "board/Piece.hpp"
#include "common/Position.hpp"

namespace kungfu {

struct PremoveData {
    Position from;
    Position to;
};

// חתימת הבדיקה: האם הכלי הנתון עסוק כרגע (בתנועה, באוויר, או בצינון)
// המחלקה עצמה לא יודעת דבר על Arbiter/Cooldown - זו רק שאילתה חיצונית שמוזרקת
using PieceBusyPredicate = std::function<bool(const PiecePtr&)>;

// חתימת ביצוע המהלך בפועל, בדרך כלל GameEngine::requestMove
using MoveExecutor = std::function<void(const Position& from, const Position& to)>;

// אחראית בלעדית על ניהול תור ה-premoves: רישום, עדכון, ניקוי כלים שנשבו,
// והרצה אוטומטית כשהכלי מתפנה. אינה יודעת דבר על זמן, תור, או חוקי משחק.
class PremoveQueue {
public:
    // רושמת premove חדש, או מעדכנת premove קיים לאותו כלי
    void registerOrUpdate(const PiecePtr& piece, const Position& from, const Position& to);

    // עוברת על כל הרישומים; עבור כל כלי שכבר לא עסוק (לפי הפרדיקט) מבצעת את המהלך
    // דרך ה-executor ומסירה את הרישום. כלים שנשבו מוסרים בלי ביצוע.
    void processReady(const PieceBusyPredicate& isBusy, const MoveExecutor& execute);

    bool empty() const noexcept { return entries_.empty(); }
    size_t size() const noexcept { return entries_.size(); }

private:
    std::vector<std::pair<PiecePtr, PremoveData>> entries_;
};

}  // namespace kungfu