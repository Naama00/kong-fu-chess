#pragma once

#include <optional>
#include "engine/common/Position.hpp"
#include "players/human/InputConfig.hpp"   

namespace kungfu {

class BoardMapper {
public:
    // Constructs a BoardMapper with the specified cell size (in pixels).
    explicit BoardMapper(int cellSize = InputConfig::kDefaultCellSize);

    std::optional<Position> pixelToCell(int x, int y, int rows, int cols) const noexcept;

    void setCellSize(int cellSize) noexcept { cellSize_ = cellSize; }

private:
    int cellSize_;
};

}  // namespace kungfu