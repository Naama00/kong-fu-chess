#pragma once

#include <optional>
#include "common/Position.hpp"

namespace kungfu {

class BoardMapper {
public:
    explicit BoardMapper(int cellSize = 100);

    // ממירה קואורדינטות פיקסל לתא לוגי בלוח.
    // מחזירה std::nullopt במידה והקליק מחוץ לגבולות המוגדרים של הלוח.
    std::optional<Position> pixelToCell(int x, int y, int rows, int cols) const noexcept;

private:
    int cellSize_;
};

}  // namespace kungfu