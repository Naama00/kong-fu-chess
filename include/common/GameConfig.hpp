// include/common/GameConfig.hpp
#pragma once

namespace kungfu {

struct GameConfig {
    // 1. קבועים לוגיים שלעולם לא ישתנו (חוקי שחמט בסיסיים)
    static constexpr int kBoardSize = 8;
    static constexpr int kWhitePawnStartRow = 1;
    static constexpr int kBlackPawnStartRow = 6;
    static constexpr int kDefaultCellSize = 100; 

    // 2. הגדרות משחק דינמיות עם ערכי ברירת מחדל (ברי שינוי בזמן ריצה)
    int cooldownDurationMs = 2000;
    int msPerCellSpeed = 1000;
    int jumpDurationMs = 1000;
    
    bool allowSimultaneousMovement = true;
    bool enablePremoves = true;
};

}  // namespace kungfu