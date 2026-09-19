#pragma once

#include <RLGym/API/Framework.h>

#include <unordered_map>
#include <vector>
#include <string>
#include <any>
#include <concepts>

#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
#define TRACY_ALLOC(name) void* operator new(std::size_t count) { auto ptr = malloc(count); TracyAllocN(ptr, count, name);return ptr;};void operator delete(void* ptr) noexcept {TracyFreeN(ptr, name);free(ptr);};
#else
#define TRACY_ALLOC(name) ;

#endif


START_API_NS

#define AGENT_MAP(...) std::unordered_map<AgentID, __VA_ARGS__>
using SharedInfo = std::unordered_map<std::string, std::any>;
template<class T, class U>
concept Derived = std::is_base_of<U, T>::value;

/// <summary>
/// Requires the type to be hashable
/// </summary>
template <typename T>
concept Hashable = requires(const T & t) {
	{ std::hash<T>{}(t) } -> std::convertible_to<std::size_t>;
};

/// <summary>
/// The structure representing the return of the call of RLGym::Step
/// </summary>
/// <typeparam name="ObsType">Type of the observation</typeparam>
/// <typeparam name="RewardType">Type of the reward</typeparam>
/// <typeparam name="AgentID">Type of the agent ID</typeparam>
template<Hashable AgentID, typename ObsType, typename RewardType>
class EnvReturn {
public:
	EnvReturn(
		AGENT_MAP(ObsType) observations,
		AGENT_MAP(RewardType) rewards,
		AGENT_MAP(bool) terminated,
		AGENT_MAP(bool) truncated
	);
	EnvReturn(const EnvReturn&) = delete;
	EnvReturn& operator=(const EnvReturn&) = delete;

	// But allow move
	EnvReturn(EnvReturn&&) = default;
	EnvReturn& operator=(EnvReturn&&) = default;

	const AGENT_MAP(ObsType)& GetObservations() const { return m_observations; }
	const AGENT_MAP(RewardType)& GetRewards() const { return m_rewards; }
	const AGENT_MAP(bool)& GetTerminated() const { return m_terminated; }
	const AGENT_MAP(bool)& GetTruncated() const { return m_truncated; }
private:
	AGENT_MAP(ObsType) m_observations;
	AGENT_MAP(RewardType) m_rewards;
	AGENT_MAP(bool) m_terminated;
	AGENT_MAP(bool) m_truncated;
};

template<Hashable AgentID, typename ObsType, typename RewardType>
inline EnvReturn<AgentID, ObsType, RewardType>::EnvReturn(AGENT_MAP(ObsType) observations, AGENT_MAP(RewardType) rewards, AGENT_MAP(bool) terminated, AGENT_MAP(bool) truncated): m_observations(std::move(observations)), m_rewards(std::move(rewards)), m_terminated(std::move(terminated)), m_truncated(std::move(truncated))
{

}

template <Hashable AgentID, typename StateType>
class ResettableObject {
public:
	/// <summary>
	/// Function to be called each time the environment is reset. Note that this does not need to return anything,
	/// the environment will call `BuildObs` automatically after reset, so the initial observation for a policy will be
	///	constructed in the same way as every other observation.
	/// </summary>
	/// <param name="agents">List of AgentIDs for which this ObsBuilder will return an Obs</param>
	/// <param name="initialState">The initial game state of the reset environment.</param>
	/// <param name="sharedInfo">A dictionary with shared information across all config objects.</param>
	virtual void Reset(const std::vector<AgentID>& agents, const StateType& initialState, SharedInfo& sharedInfo) = 0;
};

/// <summary>
/// The observation builder. This class is responsible for building observations for each agent in the environment.
/// </summary>
/// <typeparam name="ObsType">Type of the observation</typeparam>
/// <typeparam name="StateType">Type of the state to build the observation on</typeparam>
/// <typeparam name="ObsSpaceType">Type of the space used by the observation builder, used to represent, for example, the size of the observation, or whatever can define your observation space</typeparam>
/// <typeparam name="AgentID">type of the agent ID</typeparam>
template <Hashable AgentID, typename ObsType, typename StateType, typename ObsSpaceType>
class ObsBuilder: public ResettableObject<AgentID, StateType>{
public:
	/// <summary>
	/// Function that returns the observation space type. It will be called during the initialization of the environment.
	/// </summary>
	/// <param name="agent">The agent to get the observation space of</param>
	/// <returns>The observation space of the agent</returns>
	virtual const ObsSpaceType GetObservationSpace(const AgentID& agent) = 0;

	/// <summary>
	/// Function to build observations for N agents. This is where observations will be constructed every step and
	/// every reset.This function is given the current state, and it is expected that the observations returned by this
	///	function will contain information from the perspective of each agent.This function is called only once per step.
	/// </summary>
	/// <param name="agents">List of AgentIDs for which this ObsBuilder should return an Obs</param>
	/// <param name="state">The current state of the game.</param>
	/// <param name="sharedInfo">A dictionary with shared information across all config objects.</param>
	/// <returns>A dictionary of observations, one for each AgentID in agents.</returns>
	virtual const AGENT_MAP(ObsType) BuildObs(const std::vector<AgentID>& agents, const StateType& state, SharedInfo& sharedInfo) = 0;

	TRACY_ALLOC("Observation builder")
};

/// <summary>
/// The action parser. This class is responsible for receiving actions from the agents and parsing them into a format 
/// supported by the TransitionEngine.
/// </summary>
/// <typeparam name="ActionType">The type of the action sent to the action parser</typeparam>
/// <typeparam name="EngineActionType">The type of the action returned by the action parser</typeparam>
/// <typeparam name="StateType">The state used to get info for the action</typeparam>
/// <typeparam name="ActionSpaceType">The type of space used to represent the action space</typeparam>
/// <typeparam name="AgentID">The type of the agent ID</typeparam>
template <Hashable AgentID, typename ActionType, typename EngineActionType, typename StateType, typename ActionSpaceType>
class ActionParser : public ResettableObject<AgentID, StateType> {
public:
	/// <summary>
	/// Function that returns the action space type. It will be called during the initialization of the environment.
	/// </summary>
	/// <param name="agent">The agent to get the action space of</param>
	/// <returns>The action space of the given agent</returns>
	virtual const ActionSpaceType GetActionSpace(const AgentID& agent) = 0;

	/// <summary>
	/// Function that parses actions from the action space into a format that rlgym understands.
	/// The expected return value is a numpy float array of size(n, 8) where n is the number of agents.
	///	The second dimension is indexed as follows : throttle, steer, yaw, pitch, roll, jump, boost, handbrake.
	///	The first five values are expected to be in the range[-1, 1], while the last three values should be either 0 or 1.
	/// </summary>
	/// <param name="actions">An dict of actions, as passed to the `RLGym::Step` function</param>
	/// <param name="state">The GameState object of the current state that were used to generate the actions.</param>
	/// <param name="sharedInfo"> A dictionary with shared information across all config objects.</param>
	/// <returns>The parsed actions in the rlgym format</returns>
	virtual const AGENT_MAP(EngineActionType) ParseActions(const AGENT_MAP(ActionType)& actions, const StateType& state, SharedInfo& sharedInfo) = 0;
};

/// <summary>
/// The reward function. This class is responsible for computing the reward for each agent in the environment.
/// </summary>
/// <typeparam name="StateType">The type of the state used to compute rewards</typeparam>
/// <typeparam name="RewardType">The type of the computed reward</typeparam>
/// <typeparam name="AgentID">The type of the agent ID</typeparam>
template <Hashable AgentID, typename StateType, typename RewardType>
class RewardFunction: public ResettableObject<AgentID, StateType> {
public:

	/// <summary>
	/// Function to compute the reward for a player. This function is given a player argument, and it is expected that
	/// the reward returned by this function will be for that player.
	/// </summary>
	/// <param name="agents">List of AgentIDs for which this RewardFunc should return a Reward</param>
	/// <param name="state">The current state of the game.</param>
	/// <param name="isTerminated">Current state of the done conditions provided for the termination.</param>
	/// <param name="isTruncated">Current state of the done conditions provided for the truncation.</param>
	/// <param name="sharedInfo">A dictionary with shared information across all config objects.</param>
	/// <returns>A dict of rewards, one for each AgentID in agents.</returns>
	virtual const AGENT_MAP(RewardType) GetRewards(const std::vector<AgentID>& agents, const StateType& state, const AGENT_MAP(bool)& isTerminated, const AGENT_MAP(bool)& isTruncated, SharedInfo& sharedInfo) = 0;
};

/// <summary>
/// The state mutator class. This class is responsible for modifying the state of the environment.
/// </summary>
/// <typeparam name="StateType">The type of state to modify</typeparam>
template <typename StateType>
class StateMutator {
public:
	/// <summary>
	/// Function to be called each time the environment is reset.
	/// This function should change any desired values of the State.
	///	The values within State are sent to the transition engine to set up the initial state.
	/// </summary>
	/// <param name="state">State object to be modified with desired state values.</param>
	/// <param name="sharedInfo">A dictionary with shared information across all config objects.</param>
	virtual void Apply(StateType& state, SharedInfo& sharedInfo) = 0;
};

/// <summary>
/// A termination/truncation condition. This class is responsible for determining when an episode should end for each agent.
/// </summary>
/// <typeparam name="StateType">The type of state to compute the done signals on</typeparam>
/// <typeparam name="AgentID">The type of the agent ID</typeparam>
template <Hashable AgentID, typename StateType>
class DoneCondition : public ResettableObject<AgentID, StateType> {
public:
	/// <summary>
	/// Function to determine if a game state is terminal. This will be called once per step, and must return either
	/// `True` or `False` if the current episode should be terminated at this state.
	/// </summary>
	/// <param name="agents">List of AgentIDs for which this DoneCondition should be evaluated</param>
	/// <param name="state">The current state of the game.</param>
	/// <param name="sharedInfo">A dictionary with shared information across all config objects.</param>
	/// <returns>Dict of bools representing whether the current state meets this done condition for each AgentID in agents.</returns>
	virtual const AGENT_MAP(bool) IsDone(const std::vector<AgentID>& agents, const StateType& state, SharedInfo& sharedInfo) = 0;
};

/// <summary>
/// The Transition Engine class. This class is responsible for managing the state of the environment and stepping the
/// environment forward in time.
/// </summary>
/// <typeparam name="StateType">The type of the state computed by the engine.</typeparam>
/// <typeparam name="EngineActionType">The type of actions expected to step the environment.</typeparam>
/// <typeparam name="AgentID">The type of the agent ID</typeparam>
template <Hashable AgentID, typename StateType, typename EngineActionType>
class TransitionEngine {
public:
	/// <summary>
	/// Creates a minimal State object for the state mutators to modify
	/// </summary>
	/// <returns>The minimal state for the state mutators to play with</returns>
	virtual StateType CreateBaseState() = 0;

	/// <summary>
	/// Steps the logic of the environment and updates the state according to the stepped environment
	/// </summary>
	/// <param name="actions">The actions to step the environment with</param>
	/// <param name="sharedInfo">A dictionary with shared information across all config objects.</param>
	/// <returns>The stepped environment's state representation</returns>
	virtual const StateType Step(const AGENT_MAP(EngineActionType)& actions, SharedInfo& sharedInfo) = 0;

	/// <summary>
	/// Sets the state to the desired state passed in argument, if the desired state can't be set, returns the best/closest state the engine could do.
	/// </summary>
	/// <param name="desiredState">The state mutated by the state mutators</param>
	/// <param name="sharedInfo">A dictionary with shared information across all config objects.</param>
	/// <returns>Usually returns desiredState, but if the engine couldn't set a field, it'll return what it managed to set.</returns>
	virtual const StateType SetState(const StateType& desiredState, SharedInfo& sharedInfo) = 0;

	/// <summary>
	/// Called upon RLGym::Close, this is where operations that occurs upon finishing the environment should be
	/// </summary>
	virtual void Close() = 0;

	/// <summary>
	/// A getter that gets the agents in the current state
	/// </summary>
	/// <returns>The agents of the current state</returns>
	virtual const std::vector<AgentID> GetAgents() = 0;

	/// <summary>
	/// A getter that returns the maximum amount of allowed agents in the environment
	/// </summary>
	/// <returns>The maximum amount of agents in the environment</returns>
	virtual const int GetMaxNumAgents() = 0;

	/// <summary>
	/// A getter that returns the current state of the environment
	/// </summary>
	/// <returns>The current state of the environment</returns>
	virtual const StateType& GetState() = 0;
};

/// <summary>
/// The renderer class. This class is responsible for rendering a state.
/// </summary>
/// <typeparam name="StateType">The type of state to render</typeparam>
template <typename StateType>
class Renderer {
public:
	/// <summary>
	/// Renders a state, do not call this function if you seek performance, this is made to show what the environment looks like
	/// and has no reason to exist outside of debugging/showing an agent's performance.
	/// </summary>
	/// <param name="state">The state to render</param>
	/// <param name="sharedInfo"> A dictionary with shared information across all config objects.</param>
	virtual void Render(const StateType& state, SharedInfo& sharedInfo) = 0;

	/// <summary>
	/// Called upon RLGym::Close, this is where operations that occurs upon finishing the environment should be
	/// </summary>
	virtual void Close() = 0;
};

/// <summary>
/// The shared information provider. This class is responsible for managing shared information across all config objects.
/// </summary>
/// <typeparam name="StateType">The state to compute the shared info on</typeparam>
/// <typeparam name="AgentID">The type of the agent ID</typeparam>
template <Hashable AgentID, typename StateType>
class SharedInfoProvider {
public:

	/// <summary>
	/// Function to be called before anything else each time the environment is set to a particular state,
	/// either via RLGym::SetState or RLGym::Reset.
	/// </summary>
	/// <param name="sharedInfo">The previous shared information dictionary</param>
	/// <returns>A new shared info based on the given shared info</returns>
	virtual SharedInfo Create(SharedInfo& sharedInfo) = 0;

	/// <summary>
	/// Function to be called each time the environment is set to a particular state (either via set_state or reset),
	/// right after the transition engine is called.
	/// </summary>
	/// <param name="agents">List of AgentIDs for which this SharedInfoProvider will manage the SharedInfo</param>
	/// <param name="initialState">The initial state of the environment</param>
	/// <param name="sharedInfo">The previous shared information dictionary</param>
	/// <returns>A new shared info computed from the initial state of the environment</returns>
	virtual SharedInfo SetState(const std::vector<AgentID>& agents, const StateType& initialState, SharedInfo& sharedInfo) = 0;

	/// <summary>
	/// Function to be called each time the environment is stepped, right after the transition engine is called.
	/// </summary>
	/// <param name="agents">List of AgentIDs for which this SharedInfoProvider should manage the SharedInfo</param>
	/// <param name="state">The new state of the environment</param>
	/// <param name="sharedInfo">The previous shared information dictionary</param>
	/// <returns>The new shared info computed from the current state of the environment</returns>
	virtual SharedInfo Step(const std::vector<AgentID>& agents, const StateType& state, SharedInfo& sharedInfo) = 0;
};

END_API_NS