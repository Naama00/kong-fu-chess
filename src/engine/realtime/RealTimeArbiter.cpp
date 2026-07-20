// src/realtime/RealTimeArbiter.cpp
#include "engine/realtime/RealTimeArbiter.hpp"
#include "engine/realtime/CollisionDetector.hpp"
#include "engine/rules/CollisionResolver.hpp"
#include "engine/common/GameConfig.hpp"
#include <algorithm>
#include <cmath>

namespace kungfu {

RealTimeArbiter::RealTimeArbiter(std::shared_ptr<IBoard> board, GameConfig config) noexcept
    : board_(std::move(board)), config_(std::move(config)) {}

bool RealTimeArbiter::hasActiveMotion() const noexcept {
    return !activeMotions_.empty();
}

void RealTimeArbiter::startMotion(PiecePtr piece, const Position& from, const Position& to, int currentTimeMs, int durationMs) noexcept {
    activeMotions_.emplace_back(piece, from, to, currentTimeMs, durationMs);
}

// Calculates the current slot of a moving tool based on elapsed time (interpolation).
// The tool moves one cell every msPerCell milliseconds, starting from 'from' to 'to'.
static Position interpolatePosition(const Motion& motion, int currentTimeMs, int msPerCell) noexcept {
    if (motion.from() == motion.to()) {
        return motion.from(); // No movement, return the original position
    }

    int elapsed = currentTimeMs - motion.startTime();
    if (elapsed <= 0) return motion.from();

    int duration = motion.arrivalTime() - motion.startTime();
    if (duration <= 0 || elapsed >= duration) return motion.to();

    int fromR = motion.from().row(), fromC = motion.from().col();
    int toR   = motion.to().row(),   toC   = motion.to().col();
    int dr = toR - fromR, dc = toC - fromC;
    int steps = std::max(std::abs(dr), std::abs(dc)); // Total number of steps
    if (steps == 0) return motion.from();

    // Every few milliseconds the tool takes one step.
    int msPerStep = duration / steps;
    if (msPerStep <= 0) return motion.to();

    int stepsDone = elapsed / msPerStep;
    if (stepsDone >= steps) return motion.to();

    int stepR = (dr > 0) ? 1 : (dr < 0 ? -1 : 0);
    int stepC = (dc > 0) ? 1 : (dc < 0 ? -1 : 0);
    return Position(fromR + stepR * stepsDone, fromC + stepC * stepsDone);
}

std::vector<ArrivalEvent> RealTimeArbiter::advanceTime(int ms, int& currentTimeMs, PromotionHandler promoteCallback) noexcept {
    std::vector<ArrivalEvent> events;
    currentTimeMs += ms;

    CollisionResolver resolver(board_, cooldownTracker_, config_);

    auto midRouteCollisions = CollisionDetector::detectMidRouteCollisions(activeMotions_, config_);
    for (const auto& col : midRouteCollisions) {
        if (col.winner.piece()->state() != PieceState::Captured &&
            col.loser.piece()->state() != PieceState::Captured) {
            resolver.resolveMidRouteCollision(col.winner, col.loser, currentTimeMs, events);
        }
    }

    activeMotions_.erase(
        std::remove_if(activeMotions_.begin(), activeMotions_.end(),
            [](const Motion& m) {
                // Removes trapped tools, but also tools stopped by a mid-route solution
                // (state == Idle after resolveMidRouteCollision — they are no longer in motion).
                return m.piece()->state() == PieceState::Captured ||
                       m.piece()->state() == PieceState::Idle;
            }),
        activeMotions_.end()
    );

    std::vector<Motion> dueMotions;
    std::vector<Motion> remainingMotions;
    dueMotions.reserve(activeMotions_.size());
    remainingMotions.reserve(activeMotions_.size());

    for (auto& m : activeMotions_) {
        if (currentTimeMs >= m.arrivalTime()) {
            dueMotions.push_back(std::move(m));
        } else {
            remainingMotions.push_back(std::move(m));
        }
    }
    activeMotions_ = std::move(remainingMotions);

    std::stable_sort(dueMotions.begin(), dueMotions.end(), [](const Motion& a, const Motion& b) noexcept {
        return a.arrivalTime() < b.arrivalTime();
    });

    for (const auto& motion : dueMotions) {
        if (motion.piece()->state() == PieceState::Captured) {
            continue;
        }

        if (!resolver.resolveArrival(motion, currentTimeMs, events, promoteCallback)) {
            activeMotions_.push_back(motion);
        }
    }

    // Update the positions of all active motions based on the current time, even if they haven't arrived yet.
    for (const auto& motion : activeMotions_) {
        if (motion.from() != motion.to()) {
            auto interpolated = interpolatePosition(motion, currentTimeMs, config_.msPerCellSpeed);
            motion.piece()->setPosition(interpolated);
        }
    }

    return events;
}

bool RealTimeArbiter::isOnCooldown(const PiecePtr& piece, int currentTimeMs) const noexcept {
    if (!piece) return false;
    return cooldownTracker_.isOnCooldown(piece->id(), currentTimeMs);
}

bool RealTimeArbiter::isPieceMoving(const PiecePtr& piece) const noexcept {
    if (!piece) return false;
    for (const auto& motion : activeMotions_) {
        if (motion.piece() == piece) {
            return true;
        }
    }
    return false;
}

std::optional<Motion> RealTimeArbiter::getMotionForPiece(const ConstPiecePtr& piece) const noexcept {
    if (!piece) {
        return std::nullopt;
    }
    for (const auto& motion : activeMotions_) {
        if (motion.piece() == piece) {
            return motion;
        }
    }
    return std::nullopt;
}

std::optional<PiecePtr> RealTimeArbiter::getPieceInTransitAt(const Position& pos) const noexcept {
    for (const auto& m : activeMotions_) {
        if (m.from() == pos || m.to() == pos) {
            return m.piece();
        }
    }
    return std::nullopt;
}

// Checks if a piece is currently busy (either moving, airborne, or on cooldown)
bool RealTimeArbiter::isPieceBusy(const PiecePtr& piece, int currentTimeMs) const noexcept {
    if (!piece) return false;
    return isPieceMoving(piece) || 
           piece->state() == PieceState::Airborne || 
           isOnCooldown(piece, currentTimeMs);
}

// Implements the logic for setting time and physical states of the tool 
MoveResult RealTimeArbiter::executeMove(PiecePtr piece, const Position& from, const Position& to, int currentTimeMs) noexcept {
    if (from == to) {
        piece->setState(PieceState::Airborne);
        // The tool jumps "leaving" the logical square — removed from the board until it lands.
        // Other tools can occupy its square; when it lands, it will consume them.
        board_->removePiece(piece);
        startMotion(piece, from, to, currentTimeMs, config_.jumpDurationMs);
        return {true, "jump_started"};
    } else {
        int dr = std::abs(to.row() - from.row());
        int dc = std::abs(to.col() - from.col());
        int distance = std::max(dr, dc);
        int durationMs = distance * config_.msPerCellSpeed;

        piece->setState(PieceState::Moving);
        startMotion(piece, from, to, currentTimeMs, durationMs);
        return {true, "ok"};
    }
}

float RealTimeArbiter::getCooldownProgress(const PiecePtr& piece, int currentTimeMs) const noexcept {
    if (!piece) return 0.0f;
    if (config_.cooldownDurationMs <= 0) {
        return 0.0f;
    }

    int expiresAt = cooldownTracker_.getExpiration(piece->id());
    int remaining = expiresAt - currentTimeMs;
    if (remaining <= 0) return 0.0f;
    
    // Return a normalized value between 0.0 and 1.0 representing the cooldown progress
    return static_cast<float>(remaining) / config_.cooldownDurationMs;
}

}  // namespace kungfu