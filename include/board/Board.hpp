#pragma once

#include <vector>
#include "board/IBoard.hpp"
#include "common/Enums.hpp"
#include "common/Position.hpp"
#include "pieces/Piece.hpp"

namespace kungfu {

class Board : public IBoard {
public:
    Board();

    std::optional<PiecePtr> pieceAt(const Position& position) const override;
    bool placePiece(const PiecePtr& piece, const Position& position) override;
    bool removePiece(const Position& position) override;
    bool movePiece(const Position& from, const Position& to) override;

    std::vector<PiecePtr> pieces() const;

private:
    std::vector<PiecePtr> pieces_;
};

}  // namespace kungfu
