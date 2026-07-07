#pragma once

#include <RLGym/RocketLeague/Framework.h>
#include <RLGym/RocketLeague/RocketSim/GameState.h>

#include <RLGym/API/typing.h>

using namespace RLGYM_API_NS;
using namespace RLGYM_RL_NS::RGSim;

START_RL_NS(DoneConditions)

template<typename AgentID>
class GoalCondition : public DoneCondition<AgentID, GameState<AgentID>> {
public:
	void Reset(const std::vector<AgentID> agents, GameState<AgentID>& initialState, SharedInfo& sharedInfo) {}; // Noop
	AGENT_MAP(bool) IsDone(const std::vector<AgentID> agents, GameState<AgentID>& state, SharedInfo& sharedInfo) {
		AGENT_MAP(bool) dones = {};

		for (const AgentID& agent : agents) {
			dones.emplace(agent, state.goalScored);
		}

		return dones;
	}
};

END_RL_NS