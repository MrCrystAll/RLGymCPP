#pragma once

#include <RLGym/API/typing.h>
#include <optional>

START_API_NS

template <typename AgentID, typename ObsType, typename ActionType, typename EngineActionType, typename RewardType, typename StateType, typename ObsSpaceType, typename ActionSpaceType>
class RLGym {
public:
	struct EnvReturn {
		AGENT_MAP(ObsType) observations;
		AGENT_MAP(RewardType) rewards;
		AGENT_MAP(bool) terminated;
		AGENT_MAP(bool) truncated;
	};

	RLGym(
		StateMutator<StateType>* stateMutator,
		ObsBuilder<AgentID, ObsType, StateType, ObsSpaceType>* obsBuilder,
		ActionParser<AgentID, ActionType, EngineActionType, StateType, ActionSpaceType>* actionParser,
		RewardFunction<AgentID, StateType, RewardType>* rewardFunction,
		TransitionEngine<AgentID, StateType, EngineActionType>* transitionEngine,
		std::optional<DoneCondition<AgentID, StateType>*> terminationCondition = std::nullopt,
		std::optional<DoneCondition<AgentID, StateType>*> truncationCondition = std::nullopt,
		std::optional<SharedInfoProvider<AgentID, StateType>*> sharedInfoProvider = std::nullopt,
		std::optional<Renderer<StateType>*> renderer = std::nullopt) :
		m_stateMutator(stateMutator),
		m_obsBuilder(obsBuilder),
		m_actionParser(actionParser),
		m_rewardFunction(rewardFunction),
		m_transitionEngine(transitionEngine),
		m_terminationCondition(terminationCondition),
		m_truncationCondition(truncationCondition),
		m_renderer(renderer)
	{
	};

	virtual const std::vector<AgentID> GetAgents() { return this->m_transitionEngine->GetAgents(); };
	virtual const ActionSpaceType GetActionSpace(const AgentID& agentID) { return this->m_actionParser->GetActionSpace(agentID); };
	virtual const ObsSpaceType GetObservationSpace(const AgentID& agentID) { return this->m_obsBuilder->GetObservationSpace(agentID); };


	virtual const AGENT_MAP(ActionSpaceType) GetActionSpaces() {
		std::unordered_map<AgentID, ActionSpaceType> spaces = {};

		for (const AgentID& agent : this->GetAgents()) {
			spaces.emplace(agent, this->GetActionSpace(agent));
		}

		return spaces;
	}

	virtual const AGENT_MAP(ObsSpaceType) GetObservationSpaces() {
		std::unordered_map<AgentID, ObsSpaceType> spaces = {};

		for (const AgentID& agent : this->GetAgents()) {
			spaces.emplace(agent, this->GetObservationSpace(agent));
		}

		return spaces;
	}
	virtual StateType& GetState() { return this->m_transitionEngine->GetState(); };

	virtual const AGENT_MAP(ObsType) SetState(StateType& desiredState) {
		if (this->m_sharedInfoProvider.has_value()) {
			this->m_sharedInfo = this->m_sharedInfoProvider.value()->Create(this->m_sharedInfo);
		}
		StateType state = this->m_transitionEngine->SetState(desiredState, this->m_sharedInfo);
		std::vector<AgentID> agents = this->GetAgents();
		if (this->m_sharedInfoProvider.has_value()) {
			this->m_sharedInfo = this->m_sharedInfoProvider.value()->SetState(agents, state, this->m_sharedInfo);
		}
		return this->m_obsBuilder->BuildObs(agents, state, this->m_sharedInfo);
	}

	virtual const AGENT_MAP(ObsType) Reset() {
		if (this->m_sharedInfoProvider.has_value()) {
			this->m_sharedInfo = this->m_sharedInfoProvider.value()->Create(this->m_sharedInfo);
		}
		StateType desiredState = this->m_transitionEngine->CreateBaseState();
		this->m_stateMutator->Apply(desiredState, this->m_sharedInfo);
		StateType initialState = this->m_transitionEngine->SetState(desiredState, this->m_sharedInfo);
		std::vector<AgentID> agents = this->GetAgents();

		if (this->m_sharedInfoProvider.has_value()) {
			this->m_sharedInfo = this->m_sharedInfoProvider.value()->SetState(agents, initialState, this->m_sharedInfo);
		}

		this->m_obsBuilder->Reset(agents, initialState, this->m_sharedInfo);
		this->m_actionParser->Reset(agents, initialState, this->m_sharedInfo);
		if (this->m_terminationCondition.has_value()) {
			this->m_terminationCondition.value()->Reset(agents, initialState, this->m_sharedInfo);
		}
		if (this->m_truncationCondition.has_value()) {
			this->m_truncationCondition.value()->Reset(agents, initialState, this->m_sharedInfo);
		}
		this->m_rewardFunction->Reset(agents, initialState, this->m_sharedInfo);

		return this->m_obsBuilder->BuildObs(agents, initialState, this->m_sharedInfo);
	}

	virtual const EnvReturn Step(
		AGENT_MAP(ActionType) actions
	) {
		const AGENT_MAP(EngineActionType) engine_actions = this->m_actionParser->ParseActions(actions, this->GetState(), this->m_sharedInfo);
		StateType newState = this->m_transitionEngine->Step(engine_actions, this->m_sharedInfo);
		std::vector<AgentID> agents = this->GetAgents();

		if (this->m_sharedInfoProvider.has_value()) {
			this->m_sharedInfo = this->m_sharedInfoProvider.value()->Step(agents, newState, this->m_sharedInfo);
		}

		const AGENT_MAP(ObsType) obs = this->m_obsBuilder->BuildObs(agents, newState, this->m_sharedInfo);
		AGENT_MAP(bool) isTerminated = {};
		AGENT_MAP(bool) isTruncated = {};

		if (this->m_terminationCondition.has_value()) {
			isTerminated = this->m_terminationCondition.value()->IsDone(agents, newState, this->m_sharedInfo);
		}
		else {
			for (const AgentID& agentID : agents) {
				isTerminated.emplace(agentID, false);
;			}
		}

		if (this->m_truncationCondition.has_value()) {
			isTruncated = this->m_truncationCondition.value()->IsDone(agents, newState, this->m_sharedInfo);
		}
		else {
			for (const AgentID& agentID : agents) {
				isTruncated.emplace(agentID, false);
			}
		}

		const AGENT_MAP(RewardType) rewards = this->m_rewardFunction->GetRewards(agents, newState, this->m_sharedInfo);
		
		EnvReturn result = {};

		result.observations = obs;
		result.rewards = rewards;
		result.terminated = isTerminated;
		result.truncated = isTruncated;
		
		return result;
	};

	virtual void Render() {
		if (this->m_renderer.has_value()) {
			this->m_renderer.value()->Render(this->GetState(), this->m_sharedInfo);
		}
	}

	virtual void Close() {
		this->m_transitionEngine->Close();
		if (this->m_renderer.has_value()) {
			this->m_renderer.value()->Close();
		}
	}


protected:
	StateMutator<StateType>* m_stateMutator;
	ObsBuilder<AgentID, ObsType, StateType, ObsSpaceType>* m_obsBuilder;
	ActionParser<AgentID, ActionType, EngineActionType, StateType, ActionSpaceType>* m_actionParser;
	RewardFunction<AgentID, StateType, RewardType>* m_rewardFunction;
	TransitionEngine<AgentID, StateType, EngineActionType>* m_transitionEngine;
	std::optional<DoneCondition<AgentID, StateType>*> m_terminationCondition = std::nullopt;
	std::optional<DoneCondition<AgentID, StateType>*> m_truncationCondition = std::nullopt;
	std::optional<SharedInfoProvider<AgentID, StateType>*> m_sharedInfoProvider = std::nullopt;
	std::optional<Renderer<StateType>*> m_renderer = std::nullopt;

	SharedInfo m_sharedInfo = {};
};

END_API_NS