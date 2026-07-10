#pragma once

#include <memory>
#include <optional>
#include <string>
#include "input/BoardMapper.hpp"
#include "common/Position.hpp"
#include "common/GameConfig.hpp" // הוספת ייבוא הגדרות

namespace kungfu {

class IGameEngine;

struct ControllerResult {
    bool actionTaken;
    std::string description;
};

class Controller {
public:
    // שימוש בקבוע החדש כערך ברירת מחדל
    Controller(std::shared_ptr<IGameEngine> engine, int cellSize = GameConfig::kDefaultCellSize);

    ControllerResult click(int x, int y);
    std::optional<Position> selectedPosition() const noexcept;
    void clearSelection() noexcept;

private:
    std::shared_ptr<IGameEngine> engine_;
    BoardMapper mapper_;
    std::optional<Position> selectedPosition_;
};

}  // namespace kungfu