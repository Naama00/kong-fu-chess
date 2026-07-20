#pragma once

#include <memory>
#include "engine/board/IBoard.hpp"
#include "engine/common/Position.hpp"
#include "engine/board/Piece.hpp"
#include <string>

namespace kungfu {

struct MoveValidation {
    bool isValid;
    std::string reason;
};

class RuleEngine {
public:
    explicit RuleEngine(std::shared_ptr<IBoard> board) noexcept;

    // Checks the full validity of a move for the current board and returns detailed errors if the move is invalid.
    MoveValidation validateMove(const Position& from, const Position& to) const;

    // Simulation and validation of a hypothetical move (e.g., for "en passant" captures)
    MoveValidation validateHypotheticalMove(const PiecePtr& piece, const Position& from, const Position& to) const;

private:
    std::shared_ptr<IBoard> board_;
};

}  // namespace kungfu