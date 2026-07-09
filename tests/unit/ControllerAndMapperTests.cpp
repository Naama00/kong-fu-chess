#include <catch2/catch_test_macros.hpp>
#include "input/Controller.hpp"
#include "engine/IGameEngine.hpp"
#include <vector>

namespace {

// מימוש של MockGameEngine לצרכי בדיקות מבודדות של ה-Controller
class MockGameEngine : public kungfu::IGameEngine {
public:
    MockGameEngine(int rows, int cols) : rows_(rows), cols_(cols) {}

    void setHasPiece(const kungfu::Position& pos, bool hasPiece) {
        if (hasPiece) {
            occupiedPositions_.push_back(pos);
        } else {
            auto it = std::find(occupiedPositions_.begin(), occupiedPositions_.end(), pos);
            if (it != occupiedPositions_.end()) {
                occupiedPositions_.erase(it);
            }
        }
    }

    void setMoveResponse(bool accept, const std::string& reason) {
        nextMoveResult_ = {accept, reason};
    }

    // מימוש מתודות הממשק
    kungfu::MoveResult requestMove(const kungfu::Position& from, const kungfu::Position& to) override {
        lastMoveFrom_ = from;
        lastMoveTo_ = to;
        moveCount_++;
        return nextMoveResult_;
    }

    bool hasPieceAt(const kungfu::Position& pos) const override {
        return std::find(occupiedPositions_.begin(), occupiedPositions_.end(), pos) != occupiedPositions_.end();
    }

    int getBoardRows() const override { return rows_; }
    int getBoardCols() const override { return cols_; }

    // פונקציות עזר לבדיקה עצמה (Inspection queries)
    int getMoveCount() const { return moveCount_; }
    kungfu::Position getLastMoveFrom() const { return lastMoveFrom_; }
    kungfu::Position getLastMoveTo() const { return lastMoveTo_; }

private:
    int rows_;
    int cols_;
    std::vector<kungfu::Position> occupiedPositions_;
    kungfu::MoveResult nextMoveResult_ = {true, "ok"};
    
    int moveCount_ = 0;
    kungfu::Position lastMoveFrom_;
    kungfu::Position lastMoveTo_;
};

} // namespace

TEST_CASE("BoardMapper Coordinate Calculations", "[input]") {
    kungfu::BoardMapper mapper(100); // 100 pixels per cell

    SECTION("Valid mapping within borders") {
        auto cell = mapper.pixelToCell(50, 150, 8, 8);
        REQUIRE(cell.has_value());
        REQUIRE(cell->row() == 1);
        REQUIRE(cell->col() == 0);

        cell = mapper.pixelToCell(750, 750, 8, 8);
        REQUIRE(cell.has_value());
        REQUIRE(cell->row() == 7);
        REQUIRE(cell->col() == 7);
    }

    SECTION("Invalid coordinates and boundary conditions") {
        // מחוץ לגבול הימני
        REQUIRE_FALSE(mapper.pixelToCell(800, 50, 8, 8).has_value());
        // מחוץ לגבול התחתון
        REQUIRE_FALSE(mapper.pixelToCell(50, 800, 8, 8).has_value());
        // קואורדינטות שליליות
        REQUIRE_FALSE(mapper.pixelToCell(-10, 50, 8, 8).has_value());
        REQUIRE_FALSE(mapper.pixelToCell(50, -10, 8, 8).has_value());
    }
}

TEST_CASE("Controller Selection State and Clicks Flow", "[input]") {
    auto mockEngine = std::make_shared<MockGameEngine>(8, 8);
    kungfu::Controller controller(mockEngine, 100);

    // נמקם כלי דמה במיקום (1, 1)
    mockEngine->setHasPiece(kungfu::Position(1, 1), true);

    SECTION("Initial state has no selection") {
        REQUIRE_FALSE(controller.selectedPosition().has_value());
    }

    SECTION("First click on an empty cell is ignored") {
        auto result = controller.click(250, 250); // Cell (2, 2)
        REQUIRE_FALSE(result.actionTaken);
        REQUIRE_FALSE(controller.selectedPosition().has_value());
    }

    SECTION("First click on a piece selects it") {
        auto result = controller.click(150, 150); // Cell (1, 1)
        REQUIRE(result.actionTaken);
        REQUIRE(result.description == "Piece selected");
        REQUIRE(controller.selectedPosition().has_value());
        REQUIRE(controller.selectedPosition().value() == kungfu::Position(1, 1));
    }

    SECTION("Clicking outside the board without selection does nothing") {
        auto result = controller.click(900, 50); // Out of bounds
        REQUIRE_FALSE(result.actionTaken);
        REQUIRE_FALSE(controller.selectedPosition().has_value());
    }

    SECTION("Clicking outside the board with selection cancels it") {
        // נבצע בחירה
        controller.click(150, 150);
        REQUIRE(controller.selectedPosition().has_value());

        // כעת נלחץ מחוץ ללוח
        auto result = controller.click(900, 50);
        REQUIRE(result.actionTaken);
        REQUIRE(result.description == "Selection cancelled (clicked outside board)");
        REQUIRE_FALSE(controller.selectedPosition().has_value());
    }

    SECTION("Second click on any cell requests a move and clears selection") {
        // נבחר את הכלי ב-(1, 1)
        controller.click(150, 150);
        
        // נבצע מהלך ל-(2, 2) [פיקסלים 250, 250]
        mockEngine->setMoveResponse(true, "ok");
        auto result = controller.click(250, 250);

        REQUIRE(result.actionTaken);
        REQUIRE(result.description == "Move requested: ok");
        REQUIRE_FALSE(controller.selectedPosition().has_value()); // הסימון התנקה
        REQUIRE(mockEngine->getMoveCount() == 1);
        REQUIRE(mockEngine->getLastMoveFrom() == kungfu::Position(1, 1));
        REQUIRE(mockEngine->getLastMoveTo() == kungfu::Position(2, 2));
    }

    SECTION("Second click requests move even if engine rejects it, selection is still cleared") {
        controller.click(150, 150);
        
        mockEngine->setMoveResponse(false, "illegal_piece_move");
        auto result = controller.click(250, 250);

        REQUIRE(result.actionTaken);
        REQUIRE(result.description == "Move rejected: illegal_piece_move");
        REQUIRE_FALSE(controller.selectedPosition().has_value()); // הסימון בכל זאת התנקה
    }
}