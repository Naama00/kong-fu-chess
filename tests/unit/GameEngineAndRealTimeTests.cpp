#include <catch2/catch_test_macros.hpp>
#include "board/Board.hpp"
#include "io/BoardParser.hpp"
#include "rules/RuleEngine.hpp"
#include "engine/GameEngine.hpp"

TEST_CASE("Deterministic Simulated Time Flow", "[engine]") {
    std::string boardStr = 
        ". . . .\n"
        "wR . . bK\n" // צריח ב-(1,0), מלך שחור ב-(1,3)
        ". . . .\n";
        
    auto board = kungfu::BoardParser::parse(boardStr);
    REQUIRE(board != nullptr);
    auto ruleEngine = std::make_shared<kungfu::RuleEngine>(board);
    kungfu::GameEngine game(board, ruleEngine);

    SECTION("Piece remains on source cell during movement") {
        // הזמנת מהלך של שתי משבצות ל-(1,2). משך זמן מתוכנן: 2000 מילישניות.
        auto moveRes = game.requestMove(kungfu::Position(1, 0), kungfu::Position(1, 2));
        REQUIRE(moveRes.isAccepted);

        // אחרי 1500 מילישניות - הכלי עדיין במשבצת המקור (1,0)
        game.wait(1500);
        REQUIRE(board->pieceAt(kungfu::Position(1, 0)).has_value());
        REQUIRE_FALSE(board->pieceAt(kungfu::Position(1, 2)).has_value());

        // אחרי 500 מילישניות נוספות (סה"כ 2000) - הכלי הגיע ליעד (1,2)
        game.wait(500);
        REQUIRE_FALSE(board->pieceAt(kungfu::Position(1, 0)).has_value());
        REQUIRE(board->pieceAt(kungfu::Position(1, 2)).has_value());
    }

    SECTION("Enforce single active motion limitation") {
        // נתחיל מהלך עבור הצריח
        game.requestMove(kungfu::Position(1, 0), kungfu::Position(1, 2));
        
        // ניסיון להזמין מהלך נוסף בזמן שהתנועה פעילה יידחה
        auto secondaryMove = game.requestMove(kungfu::Position(1, 0), kungfu::Position(1, 1));
        REQUIRE_FALSE(secondaryMove.isAccepted);
        REQUIRE(secondaryMove.reason == "motion_in_progress");
    }

    SECTION("Capturing the enemy King ends the game") {
        // מהלך אכילה של המלך ב-(1,3). מרחק: 3 משבצות = 3000 מילישניות.
        game.requestMove(kungfu::Position(1, 0), kungfu::Position(1, 3));
        REQUIRE_FALSE(game.isGameOver());

        // התקדמות של 3000 מילישניות מביאה לאכילה וסיום המשחק
        game.wait(3000);
        REQUIRE(game.isGameOver());

        // לאחר סיום המשחק, בקשות תנועה חדשות נדחות
        auto postGameOverMove = game.requestMove(kungfu::Position(1, 3), kungfu::Position(1, 2));
        REQUIRE_FALSE(postGameOverMove.isAccepted);
        REQUIRE(postGameOverMove.reason == "game_over");
    }

    SECTION("Piece Cooldown after Arrival") {
    // מהלך מהיר של משבצת אחת ל-(1,1) -> משך תנועה: 1000ms.
    auto moveRes = game.requestMove(kungfu::Position(1, 0), kungfu::Position(1, 1));
    REQUIRE(moveRes.isAccepted);

    // המתנה של 1000ms להגעת הכלי ליעד
    game.wait(1000);
    REQUIRE(board->pieceAt(kungfu::Position(1, 1)).has_value());

    // 1. ניסיון תנועה מיידי מיד לאחר ההגעה (0ms מתום הצינון) -> נדחה
    auto quickMove = game.requestMove(kungfu::Position(1, 1), kungfu::Position(1, 2));
    REQUIRE_FALSE(quickMove.isAccepted);
    REQUIRE(quickMove.reason == "piece_on_cooldown");

    // 2. המתנה חלקית של 1500ms (סך הכל פחות מ-2000ms צינון) -> עדיין נדחה
    game.wait(1500);
    auto partialWaitMove = game.requestMove(kungfu::Position(1, 1), kungfu::Position(1, 2));
    REQUIRE_FALSE(partialWaitMove.isAccepted);
    REQUIRE(partialWaitMove.reason == "piece_on_cooldown");

    // 3. השלמת זמן הצינון (עוד 500ms, סה"כ 2000ms מרגע ההגעה) -> המהלך מאושר
    game.wait(500);
    auto afterCooldownMove = game.requestMove(kungfu::Position(1, 1), kungfu::Position(1, 2));
    REQUIRE(afterCooldownMove.isAccepted);
}

TEST_CASE("Simultaneous Movement and Cancellation on Block", "[engine]") {
    std::string boardStr = 
        "wR . . . .\n"
        ". . . . .\n"
        "wB . . . .\n"; // צריח ב-(0,0), רץ ב-(2,0)
        
    auto board = kungfu::BoardParser::parse(boardStr);
    REQUIRE(board != nullptr);
    auto ruleEngine = std::make_shared<kungfu::RuleEngine>(board);
    kungfu::GameEngine game(board, ruleEngine);

    // 1. זמן t=0: הזמנת מהלך לצריח מ-(0,0) ל-(0,2) [משך: 2000ms]
    auto rookMove = game.requestMove(kungfu::Position(0, 0), kungfu::Position(0, 2));
    REQUIRE(rookMove.isAccepted);

    // 2. זמן t=500: הזמנת מהלך לרץ מ-(2,0) ל-(0,2) [משך: 2000ms, צפוי להגיע ב-t=2500]
    // המהלך מאושר כי בזמן t=500 המשבצת (0,2) עדיין ריקה לוגית!
    game.wait(500);
    auto bishopMove = game.requestMove(kungfu::Position(2, 0), kungfu::Position(0, 2));
    REQUIRE(bishopMove.isAccepted);

    // 3. זמן t=2000 (המתנה של עוד 1500ms): הצריח מגיע ל-(0,2) ומאכלס אותה
    game.wait(1500);
    REQUIRE(board->pieceAt(kungfu::Position(0, 2)).has_value());
    REQUIRE(board->pieceAt(kungfu::Position(0, 2)).value()->type() == kungfu::PieceType::Rook);

    // 4. זמן t=2500 (המתנה של עוד 500ms): הרץ מגיע ליעד אך מוצא אותו תפוס על ידי הצריח הידידותי
    game.wait(500);
    
    // הרץ היה אמור להגיע ל-(0,2), אך מאחר והמהלך בוטל, הוא נשאר ב-(2,0) ומצבו חזר ל-Idle
    REQUIRE(board->pieceAt(kungfu::Position(2, 0)).has_value());
    REQUIRE(board->pieceAt(kungfu::Position(2, 0)).value()->type() == kungfu::PieceType::Bishop);
    REQUIRE(board->pieceAt(kungfu::Position(2, 0)).value()->state() == kungfu::PieceState::Idle);
}

}