#pragma once

#include <RLGym/API/Framework.h>

#include <unordered_map>
#include <vector>
#include <string>
#include <any>
#include <concepts>

START_API_NS

#define AGENT_MAP(_VT) std::unordered_map<AgentID, _VT>
using SharedInfo = std::unordered_map<std::string, std::any>;
template<class T, class U>
concept Derived = std::is_base_of<U, T>::value;

template <typename AgentID, typename StateType>
class ResettableObject {
public:
	virtual void Reset(const std::vector<AgentID> agents, StateType& initialState, SharedInfo& sharedInfo) = 0;
};

template <typename AgentID, typename ObsType, typename StateType, typename ObsSpaceType>
class ObsBuilder: public ResettableObject<AgentID, StateType>{
public:
	virtual ObsSpaceType GetObservationSpace(const AgentID& agent) = 0;
	virtual AGENT_MAP(ObsType) BuildObs(const std::vector<AgentID> agents, StateType& state, SharedInfo& sharedInfo) = 0;
	
};

template <typename AgentID, typename ActionType, typename EngineActionType, typename StateType, typename ActionSpaceType>
class ActionParser : public ResettableObject<AgentID, StateType> {
public:
	virtual ActionSpaceType GetActionSpace(const AgentID& agent) = 0;
	virtual AGENT_MAP(EngineActionType) ParseActions(const AGENT_MAP(ActionType) actions, StateType& state, SharedInfo& sharedInfo) = 0;
};

template <typename AgentID, typename StateType, typename RewardType>
class RewardFunction: public ResettableObject<AgentID, StateType> {
public:
	virtual AGENT_MAP(RewardType) GetRewards(const std::vector<AgentID> agents, StateType& state, SharedInfo& sharedInfo) = 0;
};

template < typename StateType>
class StateMutator {
public:
	virtual void Apply(StateType& state, SharedInfo& sharedInfo) = 0;
};

template <typename AgentID, typename StateType>
class DoneCondition : public ResettableObject<AgentID, StateType> {
public:
	virtual AGENT_MAP(bool) IsDone(const std::vector<AgentID> agents, StateType& state, SharedInfo& sharedInfo) = 0;
};

template <typename AgentID, typename StateType, typename EngineActionType>
class TransitionEngine {
public:
	virtual StateType CreateBaseState() = 0;
	virtual StateType Step(const AGENT_MAP(EngineActionType) actions, SharedInfo& sharedInfo) = 0;
	virtual StateType SetState(StateType& desiredState, SharedInfo& sharedInfo) = 0;
	virtual void Close() = 0;

	virtual const std::vector<AgentID> GetAgents() = 0;
	virtual int GetMaxNumAgents() = 0;
	virtual StateType& GetState() = 0;
};

template <typename StateType>
class Renderer {
public:
	virtual void Render(StateType& state, SharedInfo& sharedInfo) = 0;
	virtual void Close() = 0;
};

template <typename AgentID, typename StateType>
class SharedInfoProvider {
public:
	virtual SharedInfo Create(SharedInfo& sharedInfo) = 0;
	virtual SharedInfo SetState(std::vector<AgentID>, StateType& initialState, SharedInfo& sharedInfo) = 0;
	virtual SharedInfo Step(std::vector<AgentID>, StateType& state, SharedInfo& sharedInfo) = 0;
};

END_API_NS