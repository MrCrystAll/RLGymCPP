#pragma once

#include <RLGym/RocketLeague/Framework.h>
#include <RLGym/RocketLeague/RocketSim/typing.h>
#include <RLGym/RocketLeague/RocketSim/GameState.h>

#include <RLGym/API/typing.h>

#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
#endif

using namespace RLGYM_API_NS;
using namespace RLGYM_RL_NS::RGSim;

START_RL_NS(ActionParsers)

/// <summary>
/// A simple wrapper to emulate tick skip.

/// Repeats every action for a specified number of ticks.
/// </summary>
/// <typeparam name="ActionType">The type of action the parser being repeated by this class uses</typeparam>
/// <typeparam name="ActionSpaceType">The type of action space the parser being repeated by this class has</typeparam>
/// <typeparam name="AgentID">The type of the agent ID</typeparam>
template<Hashable AgentID, typename ActionType, typename ActionSpaceType>
class RepeatAction : public ActionParser<AgentID, ActionType, RocketSimAction, GameState<AgentID>, ActionSpaceType> {
public:
	RepeatAction(ActionParser<AgentID, ActionType, RocketSimAction, GameState<AgentID>, ActionSpaceType>* innerParser, int repeats = 8) : m_innerParser(innerParser), m_repeats(repeats) {};

	const ActionSpaceType GetActionSpace(const AgentID& agent) override { return this->m_innerParser->GetActionSpace(agent); };
	const AGENT_MAP(RocketSimAction) ParseActions(const AGENT_MAP(ActionType)& actions, const GameState<AgentID>& state, SharedInfo& sharedInfo) override {
#ifdef TRACY_ENABLE
		ZoneScopedNC("Action parsing", tracy::Color::Pink);
#endif
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
	void Reset(const std::vector<AgentID>& agents, const GameState<AgentID>& initialState, SharedInfo& sharedInfo) override {
		this->m_innerParser->Reset(agents, initialState, sharedInfo);
	}

	TRACY_ALLOC("Repeat action parser")

private:
	ActionParser<AgentID, ActionType, RocketSimAction, GameState<AgentID>, ActionSpaceType>* m_innerParser;
	int m_repeats;
};

END_RL_NS