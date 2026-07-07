#pragma once

#include <RLGym/RocketLeague/Framework.h>
#include <RLGym/RocketLeague/RocketSim/typing.h>
#include <RLGym/RocketLeague/RocketSim/GameState.h>

#include <RLGym/API/typing.h>

using namespace RLGYM_API_NS;
using namespace RLGYM_RL_NS::RGSim;

START_RL_NS(ActionParsers)

template<typename AgentID, typename ActionType, typename ActionSpaceType>
class RepeatAction : public ActionParser<AgentID, ActionType, RocketSimAction, GameState<AgentID>, ActionSpaceType> {
public:
	RepeatAction(ActionParser<AgentID, ActionType, RocketSimAction, GameState<AgentID>, ActionSpaceType>* innerParser, int repeats = 8) : m_innerParser(innerParser), m_repeats(repeats) {};

	ActionSpaceType GetActionSpace(const AgentID& agent) override { return this->m_innerParser->GetActionSpace(agent); };
	AGENT_MAP(RocketSimAction) ParseActions(const AGENT_MAP(ActionType) actions, GameState<AgentID>& state, SharedInfo& sharedInfo) override {
		AGENT_MAP(RocketSimAction) parsedActions = {};

		for (const auto& [agent, action] : this->m_innerParser->ParseActions(actions, state, sharedInfo)) {
			RocketSimAction parsedAction = {};
			for (int i = 0; i < this->m_repeats; i++) {
				parsedAction.append_range(action);
			}
			parsedActions.emplace(agent, parsedAction);
		}

		return parsedActions;
	};
	void Reset(const std::vector<AgentID> agents, GameState<AgentID>& initialState, SharedInfo& sharedInfo) override {
		this->m_innerParser->Reset(agents, initialState, sharedInfo);
	}

private:
	ActionParser<AgentID, ActionType, RocketSimAction, GameState<AgentID>, ActionSpaceType>* m_innerParser;
	int m_repeats;
};

END_RL_NS