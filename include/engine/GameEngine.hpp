#pragma once

#include "engine/IGameEngine.hpp"
#include "board/IBoard.hpp"
#include "rules/RuleEngine.hpp"
#include "realtime/RealTimeArbiter.hpp"
#include <vector>

namespace kungfu {

struct PremoveData {
    Position from;
    Position to;
};

class GameEngine : public IGameEngine {
public:
    GameEngine(std::shared_ptr<IBoard> board, 
               std::shared_ptr<RuleEngine> ruleEngine,
               GameConfig config = GameConfig{}) noexcept;
    
    MoveResult requestMove(const Position& from, const Position& to) override;
    bool hasPieceAt(const Position& pos) const override;
    int getBoardRows() const override;
    int getBoardCols() const override;
    std::optional<PlayerColor> getPieceColorAt(const Position& pos) const override;

    void wait(int ms) noexcept;
    bool isGameOver() const noexcept { return gameOver_; }
    int getCurrentTimeMs() const noexcept { return currentTimeMs_; }
     // מאפשר לשכבת התצוגה לגשת ללוח הלוגי לקריאה בלבד
    std::shared_ptr<const IBoard> getBoard() const noexcept { return board_; }
    // מאפשר לשכבת התצוגה לגשת למנהל התנועות לקריאה בלבד
    const RealTimeArbiter& getArbiter() const noexcept { return arbiter_; }
private:
    void processPremoves() noexcept;

    // פונקציות עזר פרטיות לניהול וניתוב בקשות התנועה
    MoveResult handlePremoveRegistration(const PiecePtr& piece, const Position& from, const Position& to) noexcept;
    MoveResult handleJumpRequest(const PiecePtr& piece, const Position& pos) noexcept;
    MoveResult handleStandardMove(const PiecePtr& piece, const Position& from, const Position& to) noexcept;

    std::shared_ptr<IBoard> board_;
    std::shared_ptr<RuleEngine> ruleEngine_;
    RealTimeArbiter arbiter_;
    int currentTimeMs_ = 0;
    bool gameOver_ = false;
    GameConfig config_;    

    std::vector<std::pair<PiecePtr, PremoveData>> premoves_;
};

}  // namespace kungfu