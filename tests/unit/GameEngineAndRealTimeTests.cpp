#include <catch2/catch_test_macros.hpp>
#include "board/Board.hpp"
#include "io/BoardParser.hpp"
#include "rules/RuleEngine.hpp"
#include "engine/GameEngine.hpp"
namespace {

// חוק הכתרה מדומה לצורך בדיקה: רגלי שמגיע לשורה האחרונה לעולם לא מוכתר
class NoPromotionRule : public kungfu::IPromotionRule {
public:
    kungfu::PiecePtr maybePromote(const kungfu::PiecePtr& piece, const kungfu::Position&, kungfu::IBoard&) const override {
        return piece;
    }
};

} // namespace

TEST_CASE("GameEngine Supports Injectable Promotion Rules", "[engine][promotion]") {
    std::string boardStr =
        ". . .\n"
        "wP . .\n"
        ". . .\n";

    auto board = kungfu::BoardParser::parse(boardStr);
    auto ruleEngine = std::make_shared<kungfu::RuleEngine>(board);
    kungfu::GameConfig cfg;
    auto noPromotion = std::make_shared<NoPromotionRule>();

    kungfu::GameEngine game(board, ruleEngine, cfg, noPromotion);

    auto res = game.requestMove(kungfu::Position(1, 0), kungfu::Position(2, 0));
    REQUIRE(res.isAccepted);

    game.wait(1000);

    // עם NoPromotionRule מוזרק, הרגלי מגיע לשורה האחרונה ונשאר רגלי - לא מוכתר אוטומטית
    auto pieceOpt = board->pieceAt(kungfu::Position(2, 0));
    REQUIRE(pieceOpt.has_value());
    REQUIRE(pieceOpt.value()->type() == kungfu::PieceType::Pawn);
}
TEST_CASE("Premoves Are Disabled in Turn-Based Mode", "[engine]") {
    std::string boardStr =
        "wR . .\n"
        ". . .\n"
        ". . bR\n";

    auto board = kungfu::BoardParser::parse(boardStr);
    auto ruleEngine = std::make_shared<kungfu::RuleEngine>(board);
    kungfu::GameConfig cfg;
    cfg.allowSimultaneousMovement = false;
    cfg.enablePremoves = true; // מאופשר בקונפיג, אבל לא אמור להיות רלוונטי במצב תור-תור
    kungfu::GameEngine game(board, ruleEngine, cfg);

    // לבן מזיז את הצריח - מהלך של 2000ms
    auto whiteMove = game.requestMove(kungfu::Position(0, 0), kungfu::Position(0, 2));
    REQUIRE(whiteMove.isAccepted);

    // ניסיון לרשום "premove" לאותו כלי לבן בזמן שהוא עדיין בתנועה - אמור להידחות ישירות,
    // ולא להירשם לתור Premoves
    auto secondAttempt = game.requestMove(kungfu::Position(0, 0), kungfu::Position(0, 1));
    REQUIRE_FALSE(secondAttempt.isAccepted);
    REQUIRE(secondAttempt.reason != "premove_registered");

    // נוודא שהתור לא התקדם בעקבות ניסיון כושל
    REQUIRE(game.currentTurn() == kungfu::PlayerColor::Black);
}

TEST_CASE("Premoves Still Work Normally in Simultaneous Mode", "[engine]") {
    std::string boardStr =
        "wR . .\n"
        ". . .\n"
        ". . .\n";

    auto board = kungfu::BoardParser::parse(boardStr);
    auto ruleEngine = std::make_shared<kungfu::RuleEngine>(board);
    kungfu::GameConfig cfg; // allowSimultaneousMovement=true, enablePremoves=true כברירת מחדל
    kungfu::GameEngine game(board, ruleEngine, cfg);

    auto firstMove = game.requestMove(kungfu::Position(0, 0), kungfu::Position(0, 1));
    REQUIRE(firstMove.isAccepted);

    auto premove = game.requestMove(kungfu::Position(0, 0), kungfu::Position(0, 2));
    REQUIRE(premove.isAccepted);
    REQUIRE(premove.reason == "premove_registered");
}
TEST_CASE("Turn-Based Mode Enforcement", "[engine]") {
    std::string boardStr =
        "wR . .\n"
        ". . .\n"
        ". . bR\n";

    auto board = kungfu::BoardParser::parse(boardStr);
    auto ruleEngine = std::make_shared<kungfu::RuleEngine>(board);
    kungfu::GameConfig cfg;
    cfg.allowSimultaneousMovement = false;
    cfg.enablePremoves = false;
    kungfu::GameEngine game(board, ruleEngine, cfg);

    SECTION("White moves first by default") {
        REQUIRE(game.currentTurn() == kungfu::PlayerColor::White);
    }

    SECTION("Black cannot move out of turn") {
        auto result = game.requestMove(kungfu::Position(2, 2), kungfu::Position(2, 1));
        REQUIRE_FALSE(result.isAccepted);
        REQUIRE(result.reason == "not_your_turn");
    }

    SECTION("Successful move passes the turn to the opponent") {
        auto result = game.requestMove(kungfu::Position(0, 0), kungfu::Position(0, 1));
        REQUIRE(result.isAccepted);
        REQUIRE(game.currentTurn() == kungfu::PlayerColor::Black);
    }

    SECTION("Opponent must wait for the piece to arrive before moving") {
        game.requestMove(kungfu::Position(0, 0), kungfu::Position(0, 1)); // 1000ms
        auto blackAttempt = game.requestMove(kungfu::Position(2, 2), kungfu::Position(2, 1));
        REQUIRE_FALSE(blackAttempt.isAccepted);
        REQUIRE(blackAttempt.reason == "motion_in_progress");

        game.wait(1000);
        auto blackAttempt2 = game.requestMove(kungfu::Position(2, 2), kungfu::Position(2, 1));
        REQUIRE(blackAttempt2.isAccepted);
    }

    SECTION("A rejected move does not pass the turn") {
        auto illegalMove = game.requestMove(kungfu::Position(0, 0), kungfu::Position(1, 1));
        REQUIRE_FALSE(illegalMove.isAccepted);
        REQUIRE(game.currentTurn() == kungfu::PlayerColor::White);
    }
}

TEST_CASE("Simultaneous Mode Allows Both Colors Freely", "[engine]") {
    std::string boardStr =
        "wR . .\n"
        ". . .\n"
        ". . bR\n";

    auto board = kungfu::BoardParser::parse(boardStr);
    auto ruleEngine = std::make_shared<kungfu::RuleEngine>(board);
    kungfu::GameConfig cfg; // allowSimultaneousMovement=true כברירת מחדל
    kungfu::GameEngine game(board, ruleEngine, cfg);

    auto whiteMove = game.requestMove(kungfu::Position(0, 0), kungfu::Position(0, 1));
    REQUIRE(whiteMove.isAccepted);

    auto blackMove = game.requestMove(kungfu::Position(2, 2), kungfu::Position(2, 1));
    REQUIRE(blackMove.isAccepted); // אין צורך להמתין לתור או לנחיתת הלבן
}
TEST_CASE("Deterministic Simulated Time Flow", "[engine]") {
    std::string boardStr =
        ". . . .\n"
        "wR . . bK\n" // צריח ב-(1,0), מלך שחור ב-(1,3)
        ". . . .\n";

    auto board = kungfu::BoardParser::parse(boardStr);
    REQUIRE(board != nullptr);
    auto ruleEngine = std::make_shared<kungfu::RuleEngine>(board);
    // allowSimultaneousMovement=false: בדיקות הצינון ויחידות-התנועה דורשות מצב זה
    kungfu::GameConfig cfg;
    cfg.allowSimultaneousMovement = false;
    cfg.enablePremoves = false;
    kungfu::GameEngine game(board, ruleEngine, cfg);

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
}

TEST_CASE("Advanced Real-Time Interactions (Collisions and Premoves)", "[engine]") {
    // נוודא בקוד הבדיקה שדגלי המשחק הדרושים לזמן אמת מאופשרים
    // (הנחת עבודה: GameConfig::kAllowSimultaneousMovement = true וכן kEnablePremoves = true)

    SECTION("Premove Queue and Auto-Execution") {
        std::string boardStr =
            "wR . .\n"
            ". . .\n"
            ". . .\n"; // צריח לבן ב-(0,0)

        auto board = kungfu::BoardParser::parse(boardStr);
        REQUIRE(board != nullptr);
        auto ruleEngine = std::make_shared<kungfu::RuleEngine>(board);
        kungfu::GameEngine game(board, ruleEngine);

        // 1. נשלח מהלך ראשון: צריח זז מ-(0,0) ל-(0,1).
        // מרחק: 1 -> משך זמן: 1000ms.
        auto res1 = game.requestMove(kungfu::Position(0, 0), kungfu::Position(0, 1));
        REQUIRE(res1.isAccepted);
        REQUIRE(res1.reason == "ok");

        // 2. בזמן שהכלי בתנועה (למשל, אחרי 100ms), נרשום עבורו מהלך מראש (Premove).
        // הכלי עדיין נמצא לוגית ב-(0,0) בלוח, לכן ה-from הוא (0,0).
        // ה-premove יתבצע אחרי ההגעה ל-(0,1): מ-(0,1) ל-(0,2).
        game.wait(100);
        auto res2 = game.requestMove(kungfu::Position(0, 0), kungfu::Position(0, 2));
        REQUIRE(res2.isAccepted);
        REQUIRE(res2.reason == "premove_registered"); // המהלך נרשם בהצלחה בתור!

        // 3. נקדם את הזמן ל-1000ms (סך הכל מתחילת המהלך הראשון).
        // הכלי מגיע ל-(0,1) ומתחיל זמן צינון (Cooldown) של 2000ms (עד t=3100ms).
        game.wait(900);
        REQUIRE(board->pieceAt(kungfu::Position(0, 1)).has_value());

        // המהלך מראש עדיין לא יוצא לפועל כי הכלי נמצא כעת בצינון (Cooldown)
        REQUIRE_FALSE(board->pieceAt(kungfu::Position(0, 2)).has_value());

        // 4. נקדם חלקית את זמן הצינון (עוד 1000ms, הגענו ל-t=2000ms מתחילת המשחק).
        // הכלי עדיין בצינון, ה-Premove עדיין ממתין.
        game.wait(1000);
        REQUIRE_FALSE(board->pieceAt(kungfu::Position(0, 2)).has_value());

        // 5. נשלים את זמן הצינון (עוד 1000ms, הגענו ל-t=3000ms).
        // הצינון מסתיים! מנוע המשחק מזהה שהכלי פנוי ומפעיל אוטומטית את ה-Premove.
        // המהלך החדש מ-(0,1) ל-(0,2) יוצא לדרך (מרחק 1 -> משך תנועה 1000ms).
        game.wait(1000);
        // הכלי עדיין בדרך ל-(0,2) ולכן פיזית עדיין לא הגיע ללוח הלוגי ביעד
        REQUIRE_FALSE(board->pieceAt(kungfu::Position(0, 2)).has_value());

        // 6. נמתין 1000ms נוספים להשלמת ה-Premove (הגענו ל-t=4000ms).
        // הכלי הגיע בהצלחה ליעד הסופי שלו ב-(0,2)!
        game.wait(1000);
        REQUIRE_FALSE(board->pieceAt(kungfu::Position(0, 1)).has_value());
        REQUIRE(board->pieceAt(kungfu::Position(0, 2)).has_value());
    }

    SECTION("Mid-Route Enemy Collision Resolution") {
        std::string boardStr =
            "wR . bR\n"
            ". . .\n"
            ". . .\n"; // צריח לבן ב-(0,0), צריח שחור ב-(0,2)

        auto board = kungfu::BoardParser::parse(boardStr);
        REQUIRE(board != nullptr);
        auto ruleEngine = std::make_shared<kungfu::RuleEngine>(board);
        kungfu::GameEngine game(board, ruleEngine);

        // 1. צריח לבן יוצא לדרך מ-(0,0) ל-(0,2) בזמן t=0.
        // מרחק: 2 -> משך תנועה: 2000ms.
        auto res1 = game.requestMove(kungfu::Position(0, 0), kungfu::Position(0, 2));
        REQUIRE(res1.isAccepted);

        // 2. נמתין 500ms. הצריח הלבן כבר באמצע הדרך.
        game.wait(500);

        // 3. כעת, בזמן t=500ms, הצריח השחור ב-(0,2) יוצא לדרך לכיוון הלבן ל-(0,0).
        // מרחק: 2 -> משך תנועה: 2000ms (צפוי להגיע ב-t=2500ms).
        auto res2 = game.requestMove(kungfu::Position(0, 2), kungfu::Position(0, 0));
        REQUIRE(res2.isAccepted);

        // 4. נקדם את הזמן ב-1000ms נוספים (הגענו ל-t=1500ms).
        // בגלל שהם נעים זה מול זה באותו הסגמנט, מתרחשת התנגשות (Collision) בדרך!
        // חוק ההתנגשות: הלבן מנצח כי הוא התחיל ראשון (t=0 לעומת t=500).
        // השחור מחוסל ומוסר מהמשחק. הלבן ממשיך כרגיל ליעדו.
        game.wait(1000);

        // נוודא שהצריח השחור חוסל ולא מופיע יותר על הלוח במקור שלו
        REQUIRE_FALSE(board->pieceAt(kungfu::Position(0, 2)).has_value());

        // 5. נמתין עד לסיום התנועה של הלבן (עוד 500ms, הגענו ל-t=2000ms).
        // הלבן נוחת בבטחה ביעד שלו ב-(0,2).
        game.wait(500);
        REQUIRE(board->pieceAt(kungfu::Position(0, 2)).has_value());
        REQUIRE(board->pieceAt(kungfu::Position(0, 2)).value()->color() == kungfu::PlayerColor::White);
    }
}

TEST_CASE("Pawn Promotion to Queen on Last Row", "[engine]") {
    std::string boardStr =
        ". . .\n"
        "wP . .\n"
        ". . .\n"; // לוח 3x3. השורה האחרונה עבור לבן היא שורה 2.

    auto board = kungfu::BoardParser::parse(boardStr);
    REQUIRE(board != nullptr);
    auto ruleEngine = std::make_shared<kungfu::RuleEngine>(board);
    kungfu::GameEngine game(board, ruleEngine);

    // 1. נשלח רגלי לבן מ-(1,0) לשורה האחרונה ב-(2,0). מרחק 1 -> 1000ms.
    auto res = game.requestMove(kungfu::Position(1, 0), kungfu::Position(2, 0));
    REQUIRE(res.isAccepted);

    // 2. נריץ את שעון המשחק ב-1000ms כדי שהכלי יגיע ליעד
    game.wait(1000);

    // 3. נוודא שהרגלי ב-(2,0) הוחלף בהצלחה למלכה!
    auto pieceOpt = board->pieceAt(kungfu::Position(2, 0));
    REQUIRE(pieceOpt.has_value());
    REQUIRE(pieceOpt.value()->type() == kungfu::PieceType::Queen);
}

TEST_CASE("Advanced Real-Time Jump Mechanics (Airborne and Captures)", "[engine]") {
    SECTION("Jump and Normal Landing") {
        std::string boardStr =
            "wR . .\n"
            ". . .\n"
            ". . .\n";

        auto board = kungfu::BoardParser::parse(boardStr);
        REQUIRE(board != nullptr);
        auto ruleEngine = std::make_shared<kungfu::RuleEngine>(board);
        kungfu::GameEngine game(board, ruleEngine);

        // 1. קליק/בקשה למעבר מאותה משבצת (0,0) ל-(0,0) מייצרת קפיצה
        auto res = game.requestMove(kungfu::Position(0, 0), kungfu::Position(0, 0));
        REQUIRE(res.isAccepted);
        REQUIRE(res.reason == "jump_started");

        // המצב של הכלי הופך ל-Airborne
        auto rook = board->pieceAt(kungfu::Position(0, 0)).value();
        REQUIRE(rook->state() == kungfu::PieceState::Airborne);

        // 2. אחרי 500ms הכלי עדיין באוויר
        game.wait(500);
        REQUIRE(rook->state() == kungfu::PieceState::Airborne);

        // 3. אחרי 500ms נוספים (סך הכל 1000ms), הכלי נוחת בבטחה וחוזר ל-Idle
        game.wait(500);
        REQUIRE(rook->state() == kungfu::PieceState::Idle);
    }

    SECTION("Airborne Piece Captures Arriving Enemy") {
        std::string boardStr =
            "wR . bR\n"
            ". . .\n"
            ". . .\n"; // צריח לבן ב-(0,0), צריח שחור ב-(0,2)

        auto board = kungfu::BoardParser::parse(boardStr);
        REQUIRE(board != nullptr);
        auto ruleEngine = std::make_shared<kungfu::RuleEngine>(board);
        kungfu::GameEngine game(board, ruleEngine);

        // 1. צריח לבן ב-(0,0) מתחיל קפיצה ב-t=0 (נמשכת 1000ms)
        game.requestMove(kungfu::Position(0, 0), kungfu::Position(0, 0));

        // 2. נמתין 200ms
        game.wait(200);

        // 3. נזיז את הצריח השחור ל-(0,1) כדי שיגיע מהר יותר
        board->removePiece(kungfu::Position(0, 2));
        auto blackRook = std::make_shared<kungfu::Piece>(kungfu::PieceType::Rook, kungfu::PlayerColor::Black, kungfu::Position(0, 1));
        board->placePiece(blackRook, kungfu::Position(0, 1));
        // (הסעיף הזה מכין את הסצנה; הלוגיקה המלאה מכוסה ב-Detailed Airborne Capture Sync Test)
    }

    SECTION("Detailed Airborne Capture Sync Test") {
        std::string boardStr =
            "wR bR .\n"
            ". . .\n"
            ". . .\n"; // לבן ב-(0,0), שחור ב-(0,1)

        auto board = kungfu::BoardParser::parse(boardStr);
        REQUIRE(board != nullptr);
        auto ruleEngine = std::make_shared<kungfu::RuleEngine>(board);
        kungfu::GameEngine game(board, ruleEngine);

        // 1. שחור יוצא לדרך מ-(0,1) ל-(0,0) ב-t=0. יגיע ב-t=1000.
        game.requestMove(kungfu::Position(0, 1), kungfu::Position(0, 0));

        // 2. נמתין 200ms. הלבן מתחיל קפיצה ב-(0,0) ב-t=200. הלבן יהיה באוויר עד t=1200.
        game.wait(200);
        game.requestMove(kungfu::Position(0, 0), kungfu::Position(0, 0));

        // 3. נמתין 800ms נוספים (הגענו ל-t=1000).
        // השחור מגיע ל-(0,0), הלבן קופץ באוויר ב-(0,0) -> הלבן מחסל את השחור!
        game.wait(800);

        // השחור חוסל ולא נמצא על הלוח במקור שלו (0,1) וגם לא ביעד
        REQUIRE_FALSE(board->pieceAt(kungfu::Position(0, 1)).has_value());

        // הלבן עדיין נמצא ב-(0,0) ובמצב Airborne (יישאר באוויר עוד 200ms)
        auto whiteRook = board->pieceAt(kungfu::Position(0, 0)).value();
        REQUIRE(whiteRook->state() == kungfu::PieceState::Airborne);

        // 4. נמתין עוד 200ms (הגענו ל-t=1200) - הלבן נוחת בבטחה וחוזר ל-Idle
        game.wait(200);
        REQUIRE(whiteRook->state() == kungfu::PieceState::Idle);
    }
}

TEST_CASE("Pawn hasMoved Flag Updates Automatically Upon Arrival", "[engine]") {
    std::string boardStr =
        ". . .\n"
        "wP . .\n"
        ". . .\n"
        ". . .\n"
        ". . .\n";

    auto board = kungfu::BoardParser::parse(boardStr);
    auto ruleEngine = std::make_shared<kungfu::RuleEngine>(board);
    kungfu::GameEngine game(board, ruleEngine);

    auto pawn = board->pieceAt(kungfu::Position(1, 0)).value();
    REQUIRE_FALSE(pawn->hasMoved());

    // צעד כפול מ-(1,0) ל-(3,0) - מרחק 2 -> 2000ms
    auto res = game.requestMove(kungfu::Position(1, 0), kungfu::Position(3, 0));
    REQUIRE(res.isAccepted);

    // באמצע התנועה - עדיין לא סומן כ"זז" (הכלי טרם נחת בפועל)
    game.wait(1000);
    REQUIRE_FALSE(pawn->hasMoved());

    // אחרי ההגעה בפועל - הדגל מתעדכן
    game.wait(1000);
    REQUIRE(pawn->hasMoved());

    // וכעת צעד כפול נוסף כבר אסור, גם אם הדרך פנויה
    auto secondDoubleStep = game.requestMove(kungfu::Position(3, 0), kungfu::Position(4, 0));
    REQUIRE(secondDoubleStep.isAccepted); // צעד יחיד עדיין מותר כמובן
}

TEST_CASE("Promoted Piece Gets Its Own Fresh Cooldown, Independent of the Original Pawn", "[engine]") {
    std::string boardStr =
        ". . .\n"
        "wP . .\n"
        ". . .\n";

    auto board = kungfu::BoardParser::parse(boardStr);
    auto ruleEngine = std::make_shared<kungfu::RuleEngine>(board);
    kungfu::GameEngine game(board, ruleEngine);

    auto res = game.requestMove(kungfu::Position(1, 0), kungfu::Position(2, 0));
    REQUIRE(res.isAccepted);
    game.wait(1000); // הרגלי מגיע ומוכתר למלכה

    auto queen = board->pieceAt(kungfu::Position(2, 0)).value();
    REQUIRE(queen->type() == kungfu::PieceType::Queen);

    // המלכה החדשה בצינון מיידית - זו הזרקה תקינה של cooldown לפי ה-id שלה, לא של הרגלי
    auto immediateMove = game.requestMove(kungfu::Position(2, 0), kungfu::Position(2, 1));
    REQUIRE_FALSE(immediateMove.isAccepted);
    REQUIRE(immediateMove.reason == "piece_on_cooldown");

    // לאחר תום הצינון (2000ms) - המלכה חופשית לזוז, בזכות ה-id הנכון שלה
    game.wait(2000);
    auto afterCooldown = game.requestMove(kungfu::Position(2, 0), kungfu::Position(2, 1));
    REQUIRE(afterCooldown.isAccepted);
}

