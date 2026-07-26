#pragma once

#include <RLGym/RocketLeague/Framework.h>
#include <RLGym/RocketLeague/RocketSim/GameState.h>

#include <RLGym/API/typing.h>

using namespace RLGYM_API_NS;
using namespace RLGYM_RL_NS::RGSim;

START_RL_NS(DoneConditions)

/// <summary>
/// A DoneCondition that is satisfied when a goal is scored.
/// </summary>
/// <typeparam name="AgentID">The type of the agent ID</typeparam>
template<Hashable AgentID>
class GoalCondition : public DoneCondition<AgentID, GameState<AgentID>> {
public:
	void Reset(const std::vector<AgentID>& agents, const GameState<AgentID>& initialState, SharedInfo& sharedInfo) override {}; // Noop
	const AGENT_MAP(bool) IsDone(const std::vector<AgentID>& agents, const GameState<AgentID>& state, SharedInfo& sharedInfo) override {
		AGENT_MAP(bool) dones = {};

		for (const AgentID& agent : agents) {
			dones.emplace(agent, state.goalScored);
		}

		return dones;
	}

	TRACY_ALLOC("Goal done condition")
};

END_RL_NS