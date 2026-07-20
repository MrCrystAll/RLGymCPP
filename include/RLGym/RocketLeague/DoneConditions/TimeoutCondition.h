#pragma once

#include <RLGym/RocketLeague/Framework.h>
#include <RLGym/RocketLeague/RocketSim/GameState.h>

#include <RLGym/API/typing.h>

using namespace RLGYM_API_NS;

START_RL_NS(DoneConditions)

/// <summary>
/// A DoneCondition that triggers after an episode lasted a given amount of time
/// </summary>
/// <typeparam name="AgentID">The type of the agent ID</typeparam>
template<Hashable AgentID>
class TimeoutCondition : public DoneCondition<AgentID, RGSim::GameState<AgentID>> {
public:

	/// <summary>
	/// A DoneCondition that triggers after an episode lasted a given amount of time
	/// </summary>
	/// <param name="secondsBeforeReset">Amount of seconds before the done signal triggers</param>
	TimeoutCondition(float secondsBeforeReset) : m_ticksBeforeReset(secondsBeforeReset* TICKS_PER_SECOND) {};
	const AGENT_MAP(bool) IsDone(const std::vector<AgentID>& agents, const RGSim::GameState<AgentID>& state, SharedInfo& sharedInfo) override {
#ifdef TRACY_ENABLE
		ZoneScopedNC("Terminal condition", tracy::Color::Gray);
#endif
		this->m_currentTicks += state.tickCount - this->m_lastArenaTickCount;
		this->m_lastArenaTickCount = state.tickCount;

		bool isDone = false;

		if (this->m_currentTicks >= this->m_ticksBeforeReset) {
			isDone = true;
		}

		AGENT_MAP(bool) dones = {};
		for (const AgentID& agent : agents) {
			dones.emplace(agent, isDone);
		}

		return dones;
	}
	void Reset(const std::vector<AgentID>& agents, const RGSim::GameState<AgentID>& initialState, SharedInfo& sharedInfo) override {
		this->m_currentTicks = 0;
		this->m_lastArenaTickCount = initialState.tickCount;
	}

	TRACY_ALLOC("Timeout done condition")
private:
	int m_ticksBeforeReset, m_currentTicks = 0, m_lastArenaTickCount = 0;
};

END_RL_NS