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

template <typename T>
concept Hashable = requires(const T & t) {
	{ std::hash<T>{}(t) } -> std::convertible_to<std::size_t>;
};

template<Hashable AgentID, typename ObsType, typename RewardType>
struct EnvReturn {
	AGENT_MAP(ObsType) observations;
	AGENT_MAP(RewardType) rewards;
	AGENT_MAP(bool) terminated;
	AGENT_MAP(bool) truncated;
};

template <Hashable AgentID, typename StateType>
class ResettableObject {
public:
	virtual void Reset(const std::vector<AgentID>& agents, const StateType& initialState, SharedInfo& sharedInfo) = 0;
};

template <Hashable AgentID, typename ObsType, typename StateType, typename ObsSpaceType>
class ObsBuilder: public ResettableObject<AgentID, StateType>{
public:
	virtual const ObsSpaceType GetObservationSpace(const AgentID& agent) = 0;
	virtual const AGENT_MAP(ObsType) BuildObs(const std::vector<AgentID>& agents, const StateType& state, SharedInfo& sharedInfo) = 0;

	TRACY_ALLOC("Observation builder")
};

template <Hashable AgentID, typename ActionType, typename EngineActionType, typename StateType, typename ActionSpaceType>
class ActionParser : public ResettableObject<AgentID, StateType> {
public:
	virtual const ActionSpaceType GetActionSpace(const AgentID& agent) = 0;
	virtual const AGENT_MAP(EngineActionType) ParseActions(const AGENT_MAP(ActionType)& actions, const StateType& state, SharedInfo& sharedInfo) = 0;
};

template <Hashable AgentID, typename StateType, typename RewardType>
class RewardFunction: public ResettableObject<AgentID, StateType> {
public:
	virtual const AGENT_MAP(RewardType) GetRewards(const std::vector<AgentID>& agents, const StateType& state, SharedInfo& sharedInfo) = 0;
};

template < typename StateType>
class StateMutator {
public:
	virtual void Apply(StateType& state, SharedInfo& sharedInfo) = 0;
};

template <Hashable AgentID, typename StateType>
class DoneCondition : public ResettableObject<AgentID, StateType> {
public:
	virtual const AGENT_MAP(bool) IsDone(const std::vector<AgentID>& agents, const StateType& state, SharedInfo& sharedInfo) = 0;
};

template <Hashable AgentID, typename StateType, typename EngineActionType>
class TransitionEngine {
public:
	virtual StateType CreateBaseState() = 0;
	virtual const StateType Step(const AGENT_MAP(EngineActionType)& actions, SharedInfo& sharedInfo) = 0;
	virtual const StateType SetState(const StateType& desiredState, SharedInfo& sharedInfo) = 0;
	virtual void Close() = 0;

	virtual const std::vector<AgentID> GetAgents() = 0;
	virtual const int GetMaxNumAgents() = 0;
	virtual const StateType& GetState() = 0;
};

template <typename StateType>
class Renderer {
public:
	virtual void Render(const StateType& state, SharedInfo& sharedInfo) = 0;
	virtual void Close() = 0;
};

template <Hashable AgentID, typename StateType>
class SharedInfoProvider {
public:
	virtual SharedInfo Create(SharedInfo& sharedInfo) = 0;
	virtual SharedInfo SetState(const std::vector<AgentID>& agents, const StateType& initialState, SharedInfo& sharedInfo) = 0;
	virtual SharedInfo Step(const std::vector<AgentID>& agents, const StateType& state, SharedInfo& sharedInfo) = 0;
};

END_API_NS