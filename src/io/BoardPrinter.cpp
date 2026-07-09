#include "io/BoardPrinter.hpp"
#include <sstream>

namespace kungfu {

std::string BoardPrinter::getPieceToken(const PiecePtr& piece) {
    if (!piece) {
        return ".";
    }
    
    std::string token = (piece->color() == PlayerColor::White) ? "w" : "b";
    switch (piece->type()) {
        case PieceType::King:   token += "K"; break;
        case PieceType::Queen:  token += "Q"; break;
        case PieceType::Rook:   token += "R"; break;
        case PieceType::Bishop: token += "B"; break;
        case PieceType::Knight: token += "N"; break;
        case PieceType::Pawn:   token += "P"; break;
    }
    return token;
}

std::string BoardPrinter::print(const Board& board) {
    std::ostringstream out;
    int rows = board.rows();
    int cols = board.cols();

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            auto pieceOpt = board.pieceAt(Position(r, c));
            if (pieceOpt.has_value()) {
                out << getPieceToken(pieceOpt.value());
            } else {
                out << ".";
            }
            if (c + 1 < cols) {
                out << " ";
            }
        }
        out << "\n";
    }
    return out.str();
}

}  // namespace kungfu