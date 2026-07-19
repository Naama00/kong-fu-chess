#include "engine/core/PremoveQueue.hpp"
#include "engine/common/Enums.hpp"
#include <algorithm>

namespace kungfu {

void PremoveQueue::registerOrUpdate(const PiecePtr& piece, const Position& to) {
    auto it = std::find_if(entries_.begin(), entries_.end(), [&](const auto& pair) {
        return pair.first == piece;
    });

    if (it != entries_.end()) {
        it->second = to;
    } else {
        entries_.push_back({piece, to});
    }
}

void PremoveQueue::cancel(const PiecePtr& piece) noexcept {
    entries_.erase(
        std::remove_if(entries_.begin(), entries_.end(),
            [&](const auto& pair) { return pair.first == piece; }),
        entries_.end()
    );
}

void PremoveQueue::replacePiece(const PiecePtr& oldPiece, const PiecePtr& newPiece) noexcept {
    for (auto& entry : entries_) {
        if (entry.first == oldPiece) {
            entry.first = newPiece;
        }
    }
}

// מימוש בטוח המונע לחלוטין Iterator Invalidation
void PremoveQueue::processReady(const PieceBusyPredicate& isBusy, const MoveExecutor& execute) {
    std::vector<std::pair<PiecePtr, Position>> readyToExecute;
    std::vector<std::pair<PiecePtr, Position>> remainingEntries;

    // 1. סינון ראשוני של האיברים ללא מניפולציה ישירה של האיטרטורים בזמן הריצה
    for (const auto& entry : entries_) {
        auto piece = entry.first;

        if (!piece || piece->state() == PieceState::Captured) {
            // כלי שנלכד - נמחק אוטומטית (לא נכנס לאף אחד מהמערכים)
            continue;
        }

        if (!isBusy(piece)) {
            readyToExecute.push_back(entry);
        } else {
            remainingEntries.push_back(entry);
        }
    }

    // 2. עדכון התור המרכזי למצב היציב החדש שלו
    entries_ = std::move(remainingEntries);

    // 3. ביצוע המהלכים מתוך מערך מקומי יציב. 
    // גם אם execute יגרור רישום של pre-move חדש, הוא יתווסף בצורה בטוחה ל-entries_ החדש
    for (const auto& entry : readyToExecute) {
        if (entry.first && entry.first->state() != PieceState::Captured) {
            execute(entry.first->position(), entry.second);
        }
    }
}

}  // namespace kungfu