#pragma once

#include <RLGym/RocketLeague/Framework.h>
#include <RLGym/RocketLeague/RocketSim/GameState.h>

#include <RLGym/API/typing.h>

using namespace RLGYM_API_NS;
using namespace RLGYM_RL_NS::RGSim;

START_RL_NS(StateMutators)

/// <summary>
/// A StateMutator that applies a sequence of StateMutators to the state.
/// </summary>
/// <typeparam name="AgentID">The type of the agent ID</typeparam>
template<Hashable AgentID>
class MutatorSequence : public StateMutator<GameState<AgentID>> {
public:
	/// <summary>
	/// A StateMutator that applies a sequence of StateMutators to the state.
	/// </summary>
	/// <typeparam name="...Args">A type to represent a mutator expecting a GameState<AgentID></typeparam>
	/// <param name="...mutators">All the mutators to apply</param>
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

	TRACY_ALLOC("Mutator sequence")
private:
	std::vector<StateMutator<GameState<AgentID>>*> m_mutators;
};

END_RL_NS