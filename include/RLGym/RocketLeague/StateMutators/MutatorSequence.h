#pragma once

#include <RLGym/RocketLeague/Framework.h>
#include <RLGym/RocketLeague/RocketSim/GameState.h>

#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
#endif

#include <RLGym/API/typing.h>

using namespace RLGYM_API_NS;
using namespace RLGYM_RL_NS::RGSim;

START_RL_NS(StateMutators)

template<typename AgentID>
class MutatorSequence : public StateMutator<GameState<AgentID>> {
public:
	template<Derived<StateMutator<GameState<AgentID>>> ...Args>
	MutatorSequence(Args*... mutators) : m_mutators{ mutators... } {};

	void Apply(GameState<AgentID>& state, SharedInfo& sharedInfo) {
#ifdef TRACY_ENABLE
		ZoneScopedNC("State mutation", tracy::Color::Brown);
#endif
		for (auto& mutator : this->m_mutators) {
			mutator->Apply(state, sharedInfo);
		}
	}
private:
	std::vector<StateMutator<GameState<AgentID>>*> m_mutators;
};

END_RL_NS