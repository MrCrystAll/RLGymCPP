#pragma once

#include <RLGym/RocketLeague/Framework.h>
#include <RLGym/RocketLeague/RocketSim/GameState.h>
#include <RLGym/API/typing.h>

using namespace RLGYM_API_NS;
using namespace RLGYM_RL_NS::RGSim;

START_RL_NS(RewardFunctions)

	template <typename AgentID>
class TouchReward : public RewardFunction<AgentID, GameState<AgentID>, float> {
public:
	void Reset(const std::vector<AgentID> agents, GameState<AgentID>& initialState, SharedInfo& sharedInfo) override {}; // Noop
	AGENT_MAP(float) GetRewards(const std::vector<AgentID> agents, GameState<AgentID>& state, SharedInfo& sharedInfo) override {
		AGENT_MAP(float) rewards = {};

		for (const AgentID& agent : agents) {
			rewards.emplace(
				agent, static_cast<float>(state.cars[agent].ballTouches > 0)
			);
		}

		return rewards;
	}
};

END_RL_NS