/// Socket communication done by Martico

#pragma once

#include <RLGym/RocketLeague/Framework.h>
#include <RLGym/RocketLeague/RocketSim/GameState.h>

#include <RLGym/API/typing.h>

#include <nlohmann/json.hpp>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

using namespace RLGYM_API_NS;
using namespace nlohmann;
using namespace RLGYM_RL_NS::RGSim;

START_RL_NS(Renderers)

/// <summary>
/// A renderer that uses RocketSimVis (https://github.com/ZealanL/RocketSimVis) to display the game state
/// </summary>
/// <typeparam name="AgentID">The type of the agent ID</typeparam>
template<Hashable AgentID>
class RocketSimVisRenderer : public Renderer<GameState<AgentID>> {
public:
    RocketSimVisRenderer() {
        this->InitializeSocket();
    }

    void InitializeSocket() {
        if (socketInitialized) return;

        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "Failed to initialize Winsock" << std::endl;
            return;
        }

        m_udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (m_udpSocket == INVALID_SOCKET) {
            std::cerr << "Failed to create socket" << std::endl;
            WSACleanup();
            return;
        }

        memset(&m_serverAddr, 0, sizeof(m_serverAddr));
        m_serverAddr.sin_family = AF_INET;
        m_serverAddr.sin_port = htons(9273);
        inet_pton(AF_INET, "127.0.0.1", &m_serverAddr.sin_addr);

        socketInitialized = true;
    }

    void Render(const GameState<AgentID>& state, SharedInfo& sharedInfo) override {
        auto data = this->GameStateToJson(state).dump();
        if (this->socketInitialized) {
            sendto(
                this->m_udpSocket,
                data.c_str(),
                static_cast<int>(data.length()),
                0,
                reinterpret_cast<const sockaddr*>(&this->m_serverAddr),
                sizeof(this->m_serverAddr)
            );
        }

    };
    void Close() override {
        if (socketInitialized) {
            closesocket(m_udpSocket);
            WSACleanup();
            socketInitialized = false;
        }
    };

    TRACY_ALLOC("RocketSimVis renderer")
private:
    /*json PhysicsToJSON(const PhysicsObject& physObj) {
        json j = {};

        j["pos"] = std::span<const float, 3>(physObj.position.data(), 3);
        j["vel"] = std::span<const float, 3>(physObj.linearVelocity.data(), 3);
        j["ang_vel"] = std::span<const float, 3>(physObj.angularVelocity.data(), 3);
        j["forward"] = std::span<const float, 3>(physObj.Forward().data(), 3);
        j["up"] = std::span<const float, 3>(physObj.Up().data(), 3);
        return j;
    };*/

    json PhysicsToJSON(const PhysicsObject& physObj) {
        json j = {};

        j["position"] = std::span<const float, 3>(physObj.position.data(), 3);
        j["velocity"] = std::span<const float, 3>(physObj.linearVelocity.data(), 3);
        j["forward"] = std::span<const float, 3>(physObj.Forward().data(), 3);
        j["up"] = std::span<const float, 3>(physObj.Up().data(), 3);
        return j;
    };

    json CarToJSON(const AgentID& agent, const Car<AgentID>& car) {
        json j = {};

        j["car_id"] = agent;
        j["team_num"] = car.teamNum;
        j["phys"] = this->PhysicsToJSON(car.physics);
        j["is_demoed"] = car.IsDemoed();
        j["on_ground"] = car.OnGround();
        j["ball_touched"] = car.ballTouches > 0;
        j["has_flip"] = car.HasFlip();
        j["boost_amount"] = car.boostAmount;
        return j;
    }

    json GameStateToJson(const GameState<AgentID>& state) {
        json j = {};

        //j["gamemode"] = "soccar";
        j["ball_phys"] = this->PhysicsToJSON(state.ball);
        /*std::vector<json> players;
        for (auto& [agentId, car] : state.cars) {
            players.push_back(this->CarToJSON(agentId, car));
        }
        j["cars"] = players;

        std::vector<bool> boostPadStates = {};
        std::vector<std::span<const float, 3>> boostPadLocations = {};

        for (int i = 0; i < state.boostPads.size(); i++) {
            boostPadStates.push_back(state.boostPads[i].cooldownTimer == 0);
            boostPadLocations.push_back(std::span<const float, 3>(state.boostPads[i].location.data(), 3));
        }

        j["boost_pad_states"] = boostPadStates;
        j["boost_pad_locations"] = boostPadLocations;*/

        return j;

    }


    SOCKET m_udpSocket = INVALID_SOCKET;
    sockaddr_in m_serverAddr;
    bool socketInitialized = false;
};

END_RL_NS