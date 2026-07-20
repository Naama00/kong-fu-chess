#pragma once

#include <string>
#include "engine/board/Board.hpp"

namespace kungfu {

class BoardPrinter {
public:
    // prints the board to a string representation, where each piece is represented by its token (e.g., "wR" for white rook, "bK" for black king) and empty squares are represented by ".". Each row is separated by a newline character.
    // The output format is suitable for debugging, logging, or saving the board state in a human-readable format.
    static std::string print(const Board& board);

private:
    static std::string getPieceToken(const ConstPiecePtr& piece);
};

}  // namespace kungfu