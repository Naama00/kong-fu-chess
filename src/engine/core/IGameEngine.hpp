// include/engine/IGameEngine.hpp
#pragma once

#include <string>
#include <optional>
#include <vector>
#include "engine/actions/ActionRequest.hpp"
#include "engine/actions/ActionResult.hpp"
#include "engine/common/Position.hpp"
#include "engine/common/Enums.hpp"
#include "engine/common/MoveResult.hpp"

namespace kungfu {

class IGameEngine {
public:
    virtual ~IGameEngine() = default;

    virtual MoveResult requestMove(const Position& from, const Position& to) = 0;
    virtual std::vector<ActionResult> processActionRequests(const std::vector<ActionRequest>& requests) = 0;
    virtual bool hasPieceAt(const Position& pos) const = 0;
    virtual int getBoardRows() const = 0;
    virtual int getBoardCols() const = 0;
    virtual std::optional<PlayerColor> getPieceColorAt(const Position& pos) const = 0;
};

}  // namespace kungfu