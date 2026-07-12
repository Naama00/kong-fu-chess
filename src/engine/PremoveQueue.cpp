#include "engine/PremoveQueue.hpp"
#include "common/Enums.hpp"
#include <algorithm>

namespace kungfu {

void PremoveQueue::registerOrUpdate(const PiecePtr& piece, const Position& from, const Position& to) {
    auto it = std::find_if(entries_.begin(), entries_.end(), [&](const auto& pair) {
        return pair.first == piece;
    });

    if (it != entries_.end()) {
        it->second = {from, to};
    } else {
        entries_.push_back({piece, {from, to}});
    }
}

void PremoveQueue::processReady(const PieceBusyPredicate& isBusy, const MoveExecutor& execute) {
    for (auto it = entries_.begin(); it != entries_.end(); ) {
        auto piece = it->first;
        auto data = it->second;

        if (!piece || piece->state() == PieceState::Captured) {
            it = entries_.erase(it);
            continue;
        }

        if (!isBusy(piece)) {
            // המהלך מתבצע מהמיקום הנוכחי של הכלי (לא מ-data.from המקורי,
            // שכן הכלי כבר הגיע ליעדו הקודם עד לרגע זה)
            execute(piece->position(), data.to);
            it = entries_.erase(it);
            continue;
        }

        ++it;
    }
}

}  // namespace kungfu