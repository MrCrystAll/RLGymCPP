#pragma once

#include <numbers>
#include <algorithm>
#include <random>

#include <RLGym/RocketLeague/Framework.h>
#include <RLGym/RocketLeague/RocketSim/GameState.h>

#include <RocketSim/Math/Math.h>

#include <RLGym/API/typing.h>

using namespace RLGYM_API_NS;
using namespace RLGYM_RL_NS::RGSim;

START_RL_NS(StateMutators)

constexpr float PI = std::numbers::pi_v<float>;

const std::array<Eigen::Vector3f, 5> SPAWN_BLUE_POS = { {
    {-2048.f, -2560.f, 17.f},
    { 2048.f, -2560.f, 17.f},
    { -256.f, -3840.f, 17.f},
    {  256.f, -3840.f, 17.f},
    {    0.f, -4608.f, 17.f}
} };

const std::array<float, 5> SPAWN_BLUE_YAW = { {
    0.25f * PI,
    0.75f * PI,
    0.50f * PI,
    0.50f * PI,
    0.50f * PI
} };

const std::array<Eigen::Vector3f, 5> SPAWN_ORANGE_POS = { {
    { 2048.f, 2560.f, 17.f},
    {-2048.f, 2560.f, 17.f},
    {  256.f, 3840.f, 17.f},
    { -256.f, 3840.f, 17.f},
    {    0.f, 4608.f, 17.f}
} };

const std::array<float, 5> SPAWN_ORANGE_YAW = { {
    -0.75f * PI,
    -0.25f * PI,
    -0.50f * PI,
    -0.50f * PI,
    -0.50f * PI
} };

/// <summary>
/// A StateMutator that sets up the game state for a kickoff.
/// </summary>
/// <typeparam name="AgentID">The type of the agent ID</typeparam>
template<Hashable AgentID>
class KickoffMutator : public StateMutator<GameState<AgentID>> {
public:
    void Apply(GameState<AgentID>& state, SharedInfo& sharedInfo) {
        state.ball.position = Vector3f(0, 0, BALL_RESTING_HEIGHT);
        state.ball.linearVelocity = Vector3f::Zero();
        state.ball.angularVelocity = Vector3f::Zero();
        state.ball.SetEulerAngles(Vector3f::Zero());

        std::array<int, 5> spawnIdx = std::array<int, 5>({ 0, 1, 2, 3, 4 });

        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(spawnIdx.begin(), spawnIdx.end(), g);

        Vector3f pos = Vector3f::Zero();
        float yaw = 0;

        int blueCount = 0, orangeCount = 0;
        for (auto& [agentID, car] : state.cars) {
            if (car.teamNum == BLUE_TEAM) {
                pos = SPAWN_BLUE_POS[spawnIdx[blueCount]];
                yaw = SPAWN_BLUE_YAW[spawnIdx[blueCount]];
                blueCount++;
            }
            else {
                pos = SPAWN_ORANGE_POS[spawnIdx[orangeCount]];
                yaw = SPAWN_ORANGE_YAW[spawnIdx[orangeCount]];
                orangeCount++;
            }

            car.physics.position = pos;
            car.physics.linearVelocity = Vector3f::Zero();
            car.physics.angularVelocity = Vector3f::Zero();
            car.physics.SetEulerAngles(Vector3f(0, yaw, 0));
            car.boostAmount = 33.3;
        }
    }

    TRACY_ALLOC("Kickoff mutator")
};

END_RL_NS