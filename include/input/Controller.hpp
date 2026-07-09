#pragma once

#include <memory>
#include <optional>
#include <string>
#include "input/BoardMapper.hpp"
#include "common/Position.hpp"

namespace kungfu {

class IGameEngine;

struct ControllerResult {
    bool actionTaken;
    std::string description;
};

class Controller {
public:
    Controller(std::shared_ptr<IGameEngine> engine, int cellSize = 100);

    // מטפלת בלחיצה של משתמש בקואורדינטות פיקסל (x, y).
    ControllerResult click(int x, int y);

    // מחזירה את התא המסומן כרגע, במידה וקיים.
    std::optional<Position> selectedPosition() const noexcept;

    // מאפסת את הסימון הנוכחי.
    void clearSelection() noexcept;

private:
    std::shared_ptr<IGameEngine> engine_;
    BoardMapper mapper_;
    std::optional<Position> selectedPosition_;
};

}  // namespace kungfu