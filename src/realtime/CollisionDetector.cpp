#include "realtime/CollisionDetector.hpp"
#include <algorithm>

namespace kungfu {

// פונקציית עזר שמחזירה את רשימת המשבצות שבהן הכלי עובר (כולל start ו-end)
std::vector<Position> getDetailedPath(const Position& from, const Position& to) {
    std::vector<Position> path;
    path.push_back(from);
    
    int dr = to.row() - from.row();
    int dc = to.col() - from.col();
    if (dr == 0 && dc == 0) return path;
    
    int stepR = (dr > 0) ? 1 : ((dr < 0) ? -1 : 0);
    int stepC = (dc > 0) ? 1 : ((dc < 0) ? -1 : 0);
    
    int curR = from.row() + stepR;
    int curC = from.col() + stepC;
    
    while (curR != to.row() || curC != to.col()) {
        path.emplace_back(curR, curC);
        curR += stepR;
        curC += stepC;
    }
    path.push_back(to);
    return path;
}

std::vector<MidRouteCollision> CollisionDetector::detectMidRouteCollisions(
    const std::vector<Motion>& activeMotions,
    const GameConfig& config
) noexcept {
    std::vector<MidRouteCollision> collisions;
    if (!config.allowSimultaneousMovement || activeMotions.size() < 2) {
        return collisions;
    }

    for (size_t i = 0; i < activeMotions.size(); ++i) {
        for (size_t j = i + 1; j < activeMotions.size(); ++j) {
            const auto& m1 = activeMotions[i];
            const auto& m2 = activeMotions[j];

            // פרשים מדלגים מעל הכלים ומתנגשים רק ביעד (מטופל ב-Arrival)
            if (m1.piece()->type() == PieceType::Knight || m2.piece()->type() == PieceType::Knight) {
                continue;
            }

            // נחשב את מסלול הזמנים לכל משבצת עבור שני הכלים
            auto path1 = getDetailedPath(m1.from(), m1.to());
            auto path2 = getDetailedPath(m2.from(), m2.to());

            // חישוב כמה זמן לוקח לעבור משבצת אחת במסלול
            int duration1 = m1.arrivalTime() - m1.startTime();
            int duration2 = m2.arrivalTime() - m2.startTime();
            
            float timePerStep1 = (path1.size() > 1) ? (float)duration1 / (path1.size() - 1) : 0;
            float timePerStep2 = (path2.size() > 1) ? (float)duration2 / (path2.size() - 1) : 0;

            // נבדוק התנגשות על כל משבצת במסלולים
            for (size_t idx1 = 0; idx1 < path1.size(); ++idx1) {
                for (size_t idx2 = 0; idx2 < path2.size(); ++idx2) {
                    if (path1[idx1] == path2[idx2]) {
                        // מצאנו משבצת משותפת במסלול שלהם! 
                        // נחשב מתי כל אחד נכנס למשבצת הזו
                        int t1_enter = m1.startTime() + static_cast<int>(idx1 * timePerStep1);
                        int t2_enter = m2.startTime() + static_cast<int>(idx2 * timePerStep2);
                        
                        // זמן היציאה מהמשבצת (הכניסה למשבצת הבאה)
                        int t1_exit = (idx1 == path1.size() - 1) ? m1.arrivalTime() + 100000 : m1.startTime() + static_cast<int>((idx1 + 1) * timePerStep1);
                        int t2_exit = (idx2 == path2.size() - 1) ? m2.arrivalTime() + 100000 : m2.startTime() + static_cast<int>((idx2 + 1) * timePerStep2);

                        // אם יש חפיפה בזמנים שהם נמצאים על אותה משבצת
                        if (std::max(t1_enter, t2_enter) < std::min(t1_exit, t2_exit)) {
                            // מי שהגיע קודם למשבצת נחשב ל"נמצא שם", מי שהגיע שני מתנגש בו
                            if (t1_enter <= t2_enter) {
                                collisions.push_back({m1, m2}); // m1 היה שם קודם (או יחד), m2 מתנגש
                            } else {
                                collisions.push_back({m2, m1}); // m2 היה שם קודם, m1 מתנגש
                            }
                            // מצאנו התנגשות בין שני אלו, נעבור לזוג הבא
                            goto next_pair;
                        }
                    }
                }
            }
            next_pair:;
        }
    }
    return collisions;
}

// שאר הקוד (detectArrivals) נשאר ללא שינוי
std::vector<Motion> CollisionDetector::detectArrivals(
    const std::vector<Motion>& activeMotions,
    int currentTimeMs
) noexcept {
    std::vector<Motion> arrivals;
    for (const auto& m : activeMotions) {
        if (currentTimeMs >= m.arrivalTime()) {
            arrivals.push_back(m);
        }
    }
    return arrivals;
}

} // namespace kungfu