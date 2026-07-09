#include <catch2/catch_test_macros.hpp>
#include "board/Board.hpp"
#include "io/BoardParser.hpp"
#include "rules/RuleEngine.hpp"

TEST_CASE("RuleEngine Basic Validations", "[rules]") {
    auto board = std::make_shared<kungfu::Board>(8, 8);
    kungfu::RuleEngine engine(board);

    SECTION("Empty source error") {
        auto result = engine.validateMove(kungfu::Position(0, 0), kungfu::Position(1, 1));
        REQUIRE_FALSE(result.isValid);
        REQUIRE(result.reason == "empty_source");
    }

    SECTION("Out of board bounds error") {
        auto result = engine.validateMove(kungfu::Position(-1, 0), kungfu::Position(0, 0));
        REQUIRE_FALSE(result.isValid);
        REQUIRE(result.reason == "outside_board");
    }
}

TEST_CASE("Piece Specific Rules Integration", "[rules]") {
    
    SECTION("Rook sliding and blocking") {
        std::string boardStr = 
            ". . . . .\n"
            ". wR . bP .\n" // הצריח ב-(1,1) חסום על ידי כלי עוין ב-(1,3) ויכול לאכול אותו
            ". . . . .\n"
            ". wP . . .\n" // הצריח חסום על ידי כלי ידידותי ב-(3,1) ואינו יכול להגיע אליו או לעבור אותו
            ". . . . .\n";
            
        auto board = kungfu::BoardParser::parse(boardStr);
        REQUIRE(board != nullptr);
        kungfu::RuleEngine engine(board);

        // תנועה ימינה פנויה
        REQUIRE(engine.validateMove(kungfu::Position(1, 1), kungfu::Position(1, 2)).isValid);
        
        // אכילה של כלי עוין ביעד מותרת
        REQUIRE(engine.validateMove(kungfu::Position(1, 1), kungfu::Position(1, 3)).isValid);
        
        // מעבר מעבר לכלי עוין חסום אסור
        REQUIRE_FALSE(engine.validateMove(kungfu::Position(1, 1), kungfu::Position(1, 4)).isValid);

        // חסימה על ידי כלי ידידותי
        auto blockResult = engine.validateMove(kungfu::Position(1, 1), kungfu::Position(3, 1));
        REQUIRE_FALSE(blockResult.isValid);
        REQUIRE(blockResult.reason == "friendly_destination");
    }

    SECTION("Knight jumps over other pieces") {
        std::string boardStr = 
            "wN wP .\n"
            "wP wP .\n"
            ". . .\n";
            
        auto board = kungfu::BoardParser::parse(boardStr);
        REQUIRE(board != nullptr);
        kungfu::RuleEngine engine(board);

        // הפרש ב-(0,0) מוקף לחלוטין בכלים ידידותיים ב-(0,1), (1,0), (1,1)
        // הוא עדיין יכול לקפוץ מעליהם ל-(2,1) או (1,2)
        REQUIRE(engine.validateMove(kungfu::Position(0, 0), kungfu::Position(2, 1)).isValid);
        REQUIRE(engine.validateMove(kungfu::Position(0, 0), kungfu::Position(1, 2)).isValid);
    }

    SECTION("Simplified Pawn rules") {
        std::string boardStr = 
            ". . .\n"
            "wP . bP\n"
            ". . .\n"; // לוח בגודל 3X3. לבן ב-(1,0), שחור ב-(1,2)
            
        auto board = kungfu::BoardParser::parse(boardStr);
        REQUIRE(board != nullptr);
        kungfu::RuleEngine engine(board);

        // פ pawn לבן נע למעלה ל-(2,0)
        REQUIRE(engine.validateMove(kungfu::Position(1, 0), kungfu::Position(2, 0)).isValid);
        
        // pawn שחור נע למטה ל-(0,2)
        REQUIRE(engine.validateMove(kungfu::Position(1, 2), kungfu::Position(0, 2)).isValid);

        // אסור לזוז באלכסון אם המשבצת ריקה
        REQUIRE_FALSE(engine.validateMove(kungfu::Position(1, 0), kungfu::Position(2, 1)).isValid);
    }
}