// players/network/NetworkPlayer.hpp
#pragma once

#include <boost/asio.hpp>
#include <memory>
#include <vector>
#include <mutex>
#include <map>
#include <string>
#include <atomic>
#include <cstdint>
#include <chrono>
#include "players/IPlayer.hpp"
#include "../../server/network/NetworkMessages.hpp" 
#include "../../engine/actions/ActionRequest.hpp"
#include "../../engine/actions/ActionResult.hpp"

namespace kungfu
{
    using boost::asio::ip::udp;

    class NetworkPlayer : public IPlayer, public std::enable_shared_from_this<NetworkPlayer>
    {
    public:
        // Struct to hold serialized details of live matches running on the server
        struct ClientMatchInfo {
            std::uint64_t matchId;
            std::string whitePlayer;
            std::string blackPlayer;
        };

    private:
        boost::asio::io_context &m_ioContext;
        udp::socket m_socket;
        boost::asio::strand<boost::asio::any_io_executor> m_strand;

        std::string m_host;
        std::string m_port;

        std::atomic<std::uint64_t> m_matchId{0};
        std::atomic<PlayerColor> m_assignedColor{PlayerColor::White};
        std::atomic<bool> m_connected{false};
        std::atomic<bool> m_matchStarted{false};

        // Opponent info stored thread-safely
        std::string m_opponentUsername = "Waiting...";
        std::uint32_t m_opponentRating = 1200;
        std::mutex m_opponentInfoMutex;

        std::atomic<bool> m_matchEnded{false};
        std::atomic<bool> m_opponentDisconnected{false};

        // Shared action/result queues accessed by the UI thread (protected by m_mutex)
        std::vector<ActionRequest> m_incomingActions;
        std::vector<ActionResult> m_incomingResults;
        std::mutex m_mutex;

        std::vector<std::uint8_t> m_recvBuffer;

        std::atomic<std::uint64_t> m_nextRequestId{1};
        std::atomic<bool> m_isOpponentDisconnected{false};
        std::atomic<int> m_disconnectCountdown{20};

        // Network timers
        boost::asio::steady_timer m_heartbeatTimer;
        boost::asio::steady_timer m_retryTimer; // Reliable delivery check timer

        // Moves awaiting server confirmation (managed strictly on the Strand thread context)
        struct PendingMove {
            NetworkMovePacket packet;
            std::chrono::steady_clock::time_point lastSent;
            int retries = 0;
        };
        std::map<std::uint64_t, PendingMove> m_pendingMoves;

        bool m_isSpectator = false;
        std::uint64_t m_spectateMatchId = 0;
        std::uint64_t m_onlineRoomCode = 0;

        std::atomic<bool> m_hasPendingSync{false};
        std::string m_pendingSyncBoard;
        std::mutex m_syncMutex;

        std::vector<ClientMatchInfo> m_activeRooms;
        std::atomic<bool> m_roomsUpdated{false};
        std::mutex m_roomsMutex;

    public:
        NetworkPlayer(boost::asio::io_context &ioContext, const std::string &host, const std::string &port,
                      bool isSpectator = false, std::uint64_t spectateMatchId = 0, std::uint64_t onlineRoomCode = 0);
        ~NetworkPlayer() override;

        void connectAndJoin();
        std::vector<ActionRequest> decideActions(const view::GameSnapshot &snapshot) override;
        std::vector<ActionResult> pollResults();
        void sendMoveToServer(const PlayerAction &action);

        bool isConnected() const { return m_connected; }
        bool hasMatchStarted() const { return m_matchStarted.load(); }
        std::uint64_t matchId() const { return m_matchId; }
        PlayerColor assignedColor() const { return m_assignedColor; }
        
        std::string opponentUsername() {
            std::lock_guard<std::mutex> lock(m_opponentInfoMutex);
            return m_opponentUsername;
        }
        
        std::uint32_t opponentRating() {
            std::lock_guard<std::mutex> lock(m_opponentInfoMutex);
            return m_opponentRating;
        }

        std::uint64_t onlineRoomCode() const { return m_onlineRoomCode; }

        bool matchEnded() const { return m_matchEnded; }
        bool opponentDisconnected() const { return m_opponentDisconnected; }
        bool isOpponentDisconnectedWithCountdown() const { return m_isOpponentDisconnected.load(); }
        int opponentDisconnectCountdown() const { return m_disconnectCountdown.load(); }

        bool isSpectator() const { return m_isSpectator; }
        bool hasPendingSync() const { return m_hasPendingSync.load(); }
        std::string consumePendingSync();

        std::vector<ClientMatchInfo> getActiveRooms();
        void requestActiveRooms();

        // Publicly accessible for clean connection termination
        void handleDisconnect();

    private:
        void doConnect();
        void sendJoinRequest();
        void sendSpectateRequest(); 
        void startReceive();
        void processDatagram(std::size_t bytesRecvd);
        void writePacket(NetworkMessageType type, const std::vector<std::uint8_t> &payload);
        
        void startHeartbeat();
        
        // Application-level ACKs and reliability mechanisms
        void startRetryTimer();
        void checkAndRetryMoves();
    };
} // namespace kungfu
