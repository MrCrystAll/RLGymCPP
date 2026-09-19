#include <RLGym/api.h>
#include <RLGym/RocketLeague.h>

#include <string>
#include <vector>
#include <tuple>
#include <chrono>
#include <thread>
#include <print>

using namespace RLGYM_API_NS;
using namespace RLGYM_RL_NS;

#define Agent std::string
#define Obs std::vector<float>
#define Action int
#define EngineAction ROCKETSIM_ENGINE_ACTION
#define Reward float
#define State ROCKETSIM_STATE(Agent)
#define ObsSpace std::tuple<std::string, int>
#define ActionSpace std::tuple<std::string, int>

/*class EmptyObs : public ObsBuilder<Agent, Obs, State, ObsSpace> {
	// Inherited via ObsBuilder
	void Reset(const std::vector<Agent>& agents, const State& initialState, SharedInfo& sharedInfo) override
	{
	}
	const ObsSpace GetObservationSpace(const Agent& agent) override
	{
		return std::make_tuple("real", 0);
	}
	const std::unordered_map<Agent, Obs> BuildObs(const std::vector<Agent>& agents, const State& state, SharedInfo& sharedInfo) override
	{
		std::unordered_map<Agent, Obs> observations = {};

		for (const Agent& agent : agents) {
			observations[agent] = {};
		}

		return observations;
	}
};

class EmptyAction : public ActionParser<Agent, Action, EngineAction, State, ActionSpace> {
	// Inherited via ActionParser
	void Reset(const std::vector<Agent>& agents, const State& initialState, SharedInfo& sharedInfo) override
	{
	}
	const ActionSpace GetActionSpace(const Agent& agent) override
	{
		return std::make_tuple("discrete", 1);
	}
	const std::unordered_map<Agent, EngineAction> ParseActions(const std::unordered_map<Agent, Action>& actions, const State& state, SharedInfo& sharedInfo) override
	{
		return actions;
	}
};

class EmptyReward : public RewardFunction<Agent, State, Reward> {
	// Inherited via RewardFunction
	void Reset(const std::vector<Agent>& agents, const State& initialState, SharedInfo& sharedInfo) override
	{
	}
	const std::unordered_map<Agent, Reward> GetRewards(const std::vector<Agent>& agents, const State& state, SharedInfo& sharedInfo) override
	{
		std::unordered_map<Agent, Reward> rewards = {};

		for (const Agent& agent : agents) {
			rewards[agent] = 0;
		}

		return rewards;
	}
};

class EmptyTransitionEngine : public TransitionEngine<Agent, State, EngineAction> {
public:
	EmptyTransitionEngine() {};
	// Inherited via TransitionEngine
	State CreateBaseState() override
	{
		this->m_state = State();
		return this->m_state;
	}
	const State Step(const std::unordered_map<Agent, EngineAction>& actions, SharedInfo& sharedInfo) override
	{
		return this->m_state;
	}
	const State SetState(const State& desiredState, SharedInfo& sharedInfo) override
	{
		this->m_state = desiredState;
		return this->m_state;
	}
	void Close() override
	{
	}
	const std::vector<Agent> GetAgents() override
	{
		return { "ello" };
	}
	const int GetMaxNumAgents() override
	{
		return 1;
	}
	const State& GetState() override
	{
		return this->m_state;
	}
private:
	State m_state;
};

class EmptyDoneCondition : public DoneCondition<Agent, State> {
	// Inherited via DoneCondition
	void Reset(const std::vector<Agent>& agents, const State& initialState, SharedInfo& sharedInfo) override
	{
	}
	const std::unordered_map<Agent, bool> IsDone(const std::vector<Agent>& agents, const State& state, SharedInfo& sharedInfo) override
	{
		std::unordered_map<Agent, bool> dones = {};

		for (const Agent& agent : agents) {
			dones[agent] = false;
		}

		return dones;
	}
};

class EmptyStateMutator : public StateMutator<State> {
	// Inherited via StateMutator
	void Apply(State& state, SharedInfo& sharedInfo) override
	{
		state = 2;
	}
};
*/

int main() {
	using namespace std::chrono_literals;

	auto* env = new RLGym <
		Agent,
		Obs,
		Action,
		EngineAction,
		Reward,
		State,
		ObsSpace,
		ActionSpace
	>(
		new StateMutators::MutatorSequence<Agent>(
			new StateMutators::FixedTeamSizeMutator<Agent>(),
			new StateMutators::KickoffMutator<Agent>()
		),
		new ObsBuilders::DefaultObs<Agent>(),
		new ActionParsers::RepeatAction<Agent, Action, ActionSpace>(
			new ActionParsers::LookupTableAction<Agent>()
		),
		new RewardFunctions::TouchReward<Agent>(),
		new TransitionEngines::RocketSimEngine<Agent>(),
		new DoneConditions::TimeoutCondition<Agent>(10),
		std::nullopt,
		std::nullopt,
		new Renderers::RocketSimVisRenderer<Agent>()
	);

	const int nSteps = 100'000;
	const bool render = false;

	auto start = std::chrono::high_resolution_clock::now();

	int stepsPerSecond = 0;
	int obsBuiltPerSecond = 0;

	std::unordered_map<Agent, Action> actions = {};

	for (int i = 0; i < nSteps;) {
		env->Reset();

		actions.clear();

		bool isTerminated = false;
		bool isTruncated = false;

		while (!isTerminated and !isTruncated) {
			if (render) {
				env->Render();
				std::this_thread::sleep_for(60ms);
			}

			actions.clear();

			for (auto& agent : env->GetAgents()) {
				actions[agent] = RocketSim::Math::RandInt(0, 90);
			}

			auto& results = env->Step(actions);

			i += results->observations.size();

			stepsPerSecond++;
			obsBuiltPerSecond += results->observations.size();

			for (auto& [agent, terminated] : results->terminated) {
				if (terminated) {
					env->Reset();
					isTerminated = true;
					break;
				}
			}

			if (isTerminated) continue;

			for (auto& [agent, truncated] : results->truncated) {
				if (truncated) {
					env->Reset();
					isTruncated = true;
					break;
				}
			}
		}
	}

	env->Close();

	const auto& end = std::chrono::high_resolution_clock::now();

	auto timeSpent = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1'000.0;

	std::println("Steps per second (amount of times the environment got stepped): {}", stepsPerSecond / timeSpent);
	std::println("Ticks per second (amount of times the environment did a tick (1 step = 8 ticks with RepeatAction(8)): {}", stepsPerSecond * 8 / timeSpent);
	std::println("Obs built per second (amount of observations built in a second): {}", obsBuiltPerSecond / timeSpent);
}