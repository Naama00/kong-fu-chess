#pragma once

#include <memory>
#include "common/Enums.hpp"
#include "common/Position.hpp"

namespace kungfu {

class Piece {
public:
    Piece(PieceType type, PlayerColor color, Position position);
    virtual ~Piece() = default;

    PieceType type() const;
    PlayerColor color() const;
    Position position() const;

    void setPosition(const Position& position);

    virtual bool isMovable() const = 0;

protected:
    PieceType type_;
    PlayerColor color_;
    Position position_;
};

using PiecePtr = std::shared_ptr<Piece>;

}  // namespace kungfu
