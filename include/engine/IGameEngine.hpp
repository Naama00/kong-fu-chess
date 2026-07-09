#pragma once

#include <string>
#include "common/Position.hpp"

namespace kungfu {

struct MoveResult {
    bool isAccepted;
    std::string reason;
};

class IGameEngine {
public:
    virtual ~IGameEngine() = default;

    // הגדרת פונקציות טהורות (pure virtual)
    virtual MoveResult requestMove(const Position& from, const Position& to) = 0;
    virtual bool hasPieceAt(const Position& pos) const = 0;
    virtual int getBoardRows() const = 0;
    virtual int getBoardCols() const = 0;
};

}  // namespace kungfu