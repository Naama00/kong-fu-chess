#include "engine/io/BoardPrinter.hpp"
#include "engine/common/PieceTokenCodec.hpp"
#include <sstream>
#include <algorithm>

namespace kungfu {

// Helper function to get the token for a given piece, or "." for an empty square
std::string BoardPrinter::getPieceToken(const ConstPiecePtr& piece) {
    if (!piece) {
        return "."; // Single character for representing an empty square
    }

    std::string token = (piece->color() == PlayerColor::White) ? "w" : "b";
    token += PieceTokenCodec::toChar(piece->type());
    return token;
}

// Implementation of the main print function
std::string BoardPrinter::print(const Board& board) {
    std::ostringstream out;
    int rows = board.rows();
    int cols = board.cols();

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            auto pieceOpt = board.pieceAt(Position(r, c));
            
            if (pieceOpt.has_value()) {
                // Call the helper function to get the token for the piece
                out << getPieceToken(pieceOpt.value());
            } else {
                out << "."; // Single character for representing an empty square
            }
            
            if (c + 1 < cols) {
                out << " "; // Space between squares in the same row
            }
        }
        out << "\n";
    }
    return out.str();
}

}  // namespace kungfu