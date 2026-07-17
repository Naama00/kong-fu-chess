#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>
#include "engine/actions/ActionRequest.hpp"
#include "engine/core/IGameEngine.hpp"
#include "engine/snapshot/GameSnapshot.hpp"
#include "players/IPlayer.hpp"
#include "players/human/Controller.hpp"
#include "players/human/InputConfig.hpp"

namespace kungfu {

class HumanPlayer : public IPlayer {
public:
    HumanPlayer(std::shared_ptr<IGameEngine> engine, int cellSize = InputConfig::kDefaultCellSize);

    ControllerResult handleClick(int x, int y);
    std::optional<Position> selectedPosition() const noexcept;
    void clearSelection() noexcept;
    void setCellSize(int cellSize) noexcept;

    std::vector<ActionRequest> decideActions(const view::GameSnapshot& snapshot) override;

private:
    std::shared_ptr<IGameEngine> engine_;
    Controller controller_;
    std::vector<ActionRequest> pendingRequests_;
    std::uint64_t nextRequestId_ = 1;
};

} // namespace kungfu
