#pragma once

#include <RLGym/RocketLeague/Framework.h>
#include <RLGym/RocketLeague/RocketSim/GameState.h>

#include <RLGym/API/typing.h>

using namespace RLGYM_API_NS;

START_RL_NS(DoneConditions)

template<Hashable AgentID>
class AnyCondition : public DoneCondition<AgentID, RGSim::GameState<AgentID>> {
public:
	template<Derived<DoneCondition<AgentID, RGSim::GameState<AgentID>>> ...Args>
	AnyCondition(Args*... conditions) : m_conditions(conditions...) {};

	void Reset(const std::vector<AgentID>& agents, const GameState<AgentID>& initialState, SharedInfo& sharedInfo) override {
		for (DoneCondition<AgentID, RGSim::GameState<AgentID>>* condition : m_conditions) {
			condition->Reset(agents, initialState, sharedInfo);
		}
	};
	const AGENT_MAP(bool) IsDone(const std::vector<AgentID>& agents, const RGSim::GameState<AgentID>& state, SharedInfo& sharedInfo) override {
		bool isDone = false;
		for (DoneCondition<AgentID, RGSim::GameState<AgentID>>* condition : m_conditions) {
			const AGENT_MAP(bool)& isDoneCondition = condition->IsDone(agents, state, sharedInfo);

			for (auto& [agentID, done] : isDoneCondition) {
				isDone = done;
			}

			if (isDone) break;
		}

		AGENT_MAP(bool) dones = {};
		for (const AgentID& agent : agents) {
			dones.emplace(agent, isDone);
		}

		return dones;
	}

	TRACY_ALLOC("Any done condition")
private:
	std::vector<DoneCondition<AgentID, RGSim::GameState<AgentID>>*> m_conditions;
};

END_RL_NS