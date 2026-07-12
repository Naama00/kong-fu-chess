#include <catch2/catch_test_macros.hpp>
#include "rules/PromotionRules.hpp"
#include "board/Board.hpp"

TEST_CASE("ChessPromotionRule Behavior", "[rules][promotion]") {
    kungfu::ChessPromotionRule rule;

    SECTION("White pawn reaching the last row promotes to Queen") {
        kungfu::Board board(5, 3);
        auto pawn = std::make_shared<kungfu::Piece>(kungfu::PieceType::Pawn, kungfu::PlayerColor::White, kungfu::Position(4, 0));
        board.placePiece(pawn, kungfu::Position(4, 0));

        auto result = rule.maybePromote(pawn, kungfu::Position(4, 0), board);

        REQUIRE(result->type() == kungfu::PieceType::Queen);
        REQUIRE(result->color() == kungfu::PlayerColor::White);
        REQUIRE(board.pieceAt(kungfu::Position(4, 0)).value() == result);
    }

    SECTION("Black pawn reaching row 0 promotes to Queen") {
        kungfu::Board board(5, 3);
        auto pawn = std::make_shared<kungfu::Piece>(kungfu::PieceType::Pawn, kungfu::PlayerColor::Black, kungfu::Position(0, 0));
        board.placePiece(pawn, kungfu::Position(0, 0));

        auto result = rule.maybePromote(pawn, kungfu::Position(0, 0), board);

        REQUIRE(result->type() == kungfu::PieceType::Queen);
        REQUIRE(result->color() == kungfu::PlayerColor::Black);
    }

    SECTION("Pawn not on the last row does not promote") {
        kungfu::Board board(5, 3);
        auto pawn = std::make_shared<kungfu::Piece>(kungfu::PieceType::Pawn, kungfu::PlayerColor::White, kungfu::Position(2, 0));
        board.placePiece(pawn, kungfu::Position(2, 0));

        auto result = rule.maybePromote(pawn, kungfu::Position(2, 0), board);

        REQUIRE(result == pawn); // אותו אובייקט בדיוק, ללא שינוי
        REQUIRE(result->type() == kungfu::PieceType::Pawn);
    }

    SECTION("Non-pawn pieces never promote, even on the last row") {
        kungfu::Board board(5, 3);
        auto rook = std::make_shared<kungfu::Piece>(kungfu::PieceType::Rook, kungfu::PlayerColor::White, kungfu::Position(4, 0));
        board.placePiece(rook, kungfu::Position(4, 0));

        auto result = rule.maybePromote(rook, kungfu::Position(4, 0), board);

        REQUIRE(result == rook);
        REQUIRE(result->type() == kungfu::PieceType::Rook);
    }
}