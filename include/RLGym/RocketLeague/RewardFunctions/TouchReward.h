#pragma once

#include <RLGym/RocketLeague/Framework.h>
#include <RLGym/RocketLeague/RocketSim/GameState.h>
#include <RLGym/API/typing.h>

#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
#endif

using namespace RLGYM_API_NS;
using namespace RLGYM_RL_NS::RGSim;

START_RL_NS(RewardFunctions)

/// <summary>
/// A RewardFunction that gives a reward of 1 if the agent touches the ball, 0 otherwise.
/// </summary>
/// <typeparam name="AgentID">The type of the agent ID</typeparam>
template <Hashable AgentID>
class TouchReward : public RewardFunction<AgentID, GameState<AgentID>, float> {
public:
	void Reset(const std::vector<AgentID>& agents, const GameState<AgentID>& initialState, SharedInfo& sharedInfo) override {}; // Noop
	const AGENT_MAP(float) GetRewards(const std::vector<AgentID>& agents, const GameState<AgentID>& state, const AGENT_MAP(bool)& isTerminated, const AGENT_MAP(bool)& isTruncated, SharedInfo& sharedInfo) override {
#ifdef TRACY_ENABLE
		ZoneScopedNC("Reward computation", tracy::Color::Green);
#endif

		AGENT_MAP(float) rewards = {};

		for (const AgentID& agent : agents) {
			rewards.emplace(
				agent, static_cast<float>(state.cars.at(agent).ballTouches > 0)
			);
		}

		return rewards;
	}

	TRACY_ALLOC("Touch reward function")
};

END_RL_NS