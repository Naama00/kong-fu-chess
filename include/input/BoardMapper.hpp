#pragma once

#include <optional>
#include "common/Position.hpp"
#include "common/GameConfig.hpp" // הוספת ייבוא הגדרות

namespace kungfu {

class BoardMapper {
public:
    // שימוש בקבוע החדש כערך ברירת מחדל
    explicit BoardMapper(int cellSize = GameConfig::kDefaultCellSize);

    std::optional<Position> pixelToCell(int x, int y, int rows, int cols) const noexcept;

private:
    int cellSize_;
};

}  // namespace kungfu