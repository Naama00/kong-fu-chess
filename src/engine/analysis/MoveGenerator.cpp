#include "engine/analysis/MoveGenerator.hpp"

#include <algorithm>
#include <memory>
#include <vector>

#include "engine/board/Board.hpp"
#include "engine/common/Position.hpp"
#include "engine/rules/RuleEngine.hpp"

namespace kungfu {
namespace {

std::shared_ptr<Piece> createPieceFromSnapshot(const view::PieceSnapshot& pieceSnapshot) {
    auto piece = std::make_shared<Piece>(pieceSnapshot.type, pieceSnapshot.color, pieceSnapshot.logicalPosition);
    piece->setState(pieceSnapshot.state);
    if (pieceSnapshot.hasMoved) {
        piece->markMoved();
    }
    return piece;
}

} // namespace

std::shared_ptr<Board> MoveGenerator::buildBoardFromSnapshot(const view::GameSnapshot& snapshot) const {
    auto board = std::make_shared<Board>(snapshot.boardRows, snapshot.boardCols);
    for (const auto& pieceSnapshot : snapshot.pieces) {
        if (pieceSnapshot.type == PieceType::King && pieceSnapshot.color == PlayerColor::White) {
            // no-op placeholder to keep the function explicit for the current implementation
        }
        auto piece = createPieceFromSnapshot(pieceSnapshot);
        board->placePiece(piece, pieceSnapshot.logicalPosition);
    }
    return board;
}

std::vector<IMoveGenerator::MoveCandidate> MoveGenerator::generateForPiece(
    const view::GameSnapshot& snapshot,
    const Position& piecePosition) const {
    std::vector<MoveCandidate> result;
    auto board = buildBoardFromSnapshot(snapshot);
    if (!board) {
        return result;
    }

    auto sourcePieceOpt = board->pieceAt(piecePosition);
    if (!sourcePieceOpt.has_value() || !sourcePieceOpt.value()) {
        return result;
    }

    const auto& piece = sourcePieceOpt.value();
    RuleEngine engine(board);
    for (int row = 0; row < snapshot.boardRows; ++row) {
        for (int col = 0; col < snapshot.boardCols; ++col) {
            Position target(row, col);
            if (engine.validateMove(piecePosition, target).isValid) {
                result.push_back({piecePosition, target});
            }
        }
    }

    std::sort(result.begin(), result.end(), [](const MoveCandidate& lhs, const MoveCandidate& rhs) {
        if (lhs.to.row() != rhs.to.row()) {
            return lhs.to.row() < rhs.to.row();
        }
        return lhs.to.col() < rhs.to.col();
    });

    return result;
}

std::vector<IMoveGenerator::MoveCandidate> MoveGenerator::generateForPlayer(
    const view::GameSnapshot& snapshot,
    PlayerColor playerColor) const {
    std::vector<MoveCandidate> result;
    auto board = buildBoardFromSnapshot(snapshot);
    if (!board) {
        return result;
    }

    const auto pieces = board->pieces();
    for (const auto& piece : pieces) {
        if (!piece || piece->color() != playerColor) {
            continue;
        }
        auto pieceMoves = generateForPiece(snapshot, piece->position());
        result.insert(result.end(), pieceMoves.begin(), pieceMoves.end());
    }

    std::sort(result.begin(), result.end(), [](const MoveCandidate& lhs, const MoveCandidate& rhs) {
        if (lhs.from.row() != rhs.from.row()) {
            return lhs.from.row() < rhs.from.row();
        }
        if (lhs.from.col() != rhs.from.col()) {
            return lhs.from.col() < rhs.from.col();
        }
        if (lhs.to.row() != rhs.to.row()) {
            return lhs.to.row() < rhs.to.row();
        }
        return lhs.to.col() < rhs.to.col();
    });

    return result;
}

} // namespace kungfu
