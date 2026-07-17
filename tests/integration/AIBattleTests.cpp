// integration/AIBattleTests.cpp
#include <catch2/catch_test_macros.hpp>
#include "engine/io/BoardParser.hpp"
#include "engine/rules/RuleEngine.hpp"
#include "engine/core/GameEngine.hpp"
#include "engine/snapshot/SnapshotBuilder.hpp"
#include "players/ai/EasyAI.hpp"
#include "players/ai/HardAI.hpp"

TEST_CASE("Integration match between EasyAI and HardAI does not crash or hang", "[integration][ai]") {
    std::string boardSetup =
        "wR . . bK\n"
        ". . . .\n"
        ". . . .\n"
        ". . . .\n";

    auto board = kungfu::BoardParser::parse(boardSetup);
    REQUIRE(board != nullptr);

    auto ruleEngine = std::make_shared<kungfu::RuleEngine>(board);
    auto gameEngine = std::make_shared<kungfu::GameEngine>(board, ruleEngine);

    kungfu::EasyAI easyAI(kungfu::PlayerColor::White);
    kungfu::HardAI hardAI(kungfu::PlayerColor::Black);

    // Simulating 5 turns of AI vs AI match to prove no crashes or infinite loops exist in the minimax tree
    for (int turn = 0; turn < 5; ++turn) {
        auto snapshot = kungfu::view::SnapshotBuilder::build(
            *board,
            gameEngine->getArbiter(),
            gameEngine->getCurrentTimeMs(),
            gameEngine->isGameOver(),
            std::nullopt,
            100.0f);

        if (gameEngine->isGameOver()) {
            break;
        }

        // EasyAI decides for White
        auto whiteRequests = easyAI.decideActions(snapshot);
        if (!whiteRequests.empty()) {
            gameEngine->processActionRequests(whiteRequests);
        }

        // HardAI decides for Black
        auto blackRequests = hardAI.decideActions(snapshot);
        if (!blackRequests.empty()) {
            gameEngine->processActionRequests(blackRequests);
        }

        // Progress simulation time
        gameEngine->wait(1000);
    }

    SUCCEED("EasyAI vs HardAI match executed cleanly without timing out or crashing.");
}