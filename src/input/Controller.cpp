#include "input/Controller.hpp"
#include "engine/IGameEngine.hpp"

namespace kungfu {

Controller::Controller(std::shared_ptr<IGameEngine> engine, int cellSize)
    : engine_(std::move(engine)), mapper_(cellSize), selectedPosition_(std::nullopt) {}

ControllerResult Controller::click(int x, int y) {
    if (!engine_) {
        return {false, "Engine reference is null"};
    }

    int rows = engine_->getBoardRows();
    int cols = engine_->getBoardCols();
    auto cellOpt = mapper_.pixelToCell(x, y, rows, cols);

    // 1. קליק מחוץ לגבולות הלוח
    if (!cellOpt.has_value()) {
        if (selectedPosition_.has_value()) {
            // אם יש כלי מסומן, קליק מחוץ ללוח מבטל את הסימון
            clearSelection();
            return {true, "Selection cancelled (clicked outside board)"};
        }
        // אם אין כלי מסומן, קליק מחוץ ללוח פשוט מיועד להתעלמות
        return {false, "Click outside board ignored"};
    }

    Position targetCell = cellOpt.value();

    // 2. קליק בתוך הלוח כאשר כבר קיים כלי מסומן (קליק שני)
    if (selectedPosition_.has_value()) {
        Position from = selectedPosition_.value();
        
        // שליחת בקשת התנועה למנוע המשחק
        auto moveResult = engine_->requestMove(from, targetCell);
        
        // פינוי הסימון באופן מיידי לאחר הניסיון (לפי דרישות המדריך)
        clearSelection();

        if (moveResult.isAccepted) {
            return {true, "Move requested: " + moveResult.reason};
        } else {
            return {true, "Move rejected: " + moveResult.reason};
        }
    }

    // 3. קליק בתוך הלוח ללא כלי מסומן כרגע (קליק ראשון)
    if (engine_->hasPieceAt(targetCell)) {
        selectedPosition_ = targetCell;
        return {true, "Piece selected"};
    }

    // התעלמות מקליק ראשון על משבצת ריקה
    return {false, "Click on empty cell ignored"};
}

std::optional<Position> Controller::selectedPosition() const noexcept {
    return selectedPosition_;
}

void Controller::clearSelection() noexcept {
    selectedPosition_ = std::nullopt;
}

}  // namespace kungfu