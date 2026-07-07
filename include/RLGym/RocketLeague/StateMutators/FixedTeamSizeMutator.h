#pragma once

#include <RLGym/RocketLeague/Framework.h>
#include <RLGym/RocketLeague/RocketSim/GameState.h>

#include <RLGym/API/typing.h>

#include <assert.h>
#include <string>

using namespace RLGYM_API_NS;
using namespace RLGYM_RL_NS::RGSim;

START_RL_NS(StateMutators)

	template<typename AgentID>
class FixedTeamSizeMutator : public StateMutator<GameState<AgentID>> {
public:
	FixedTeamSizeMutator(
		int blueSize = 1, int orangeSize = 1
	) : m_blueSize(blueSize), m_orangeSize(orangeSize) {
	};

	void Apply(GameState<AgentID>& state, SharedInfo& sharedInfo) {
		assert(state.cars.size() == 0);

		for (int i = 0; i < this->m_blueSize; i++) {
			Car<AgentID> car = Car<AgentID>();
			car.teamNum = BLUE_TEAM;
			state.cars.emplace(
				std::string("blue-") + std::to_string(i),
				car
			);
		}

		for (int i = 0; i < this->m_orangeSize; i++) {
			Car<AgentID> car = Car<AgentID>();
			car.teamNum = ORANGE_TEAM;
			state.cars.emplace(
				std::string("orange-") + std::to_string(i),
				car
			);
		}
	}
protected:
	int m_blueSize, m_orangeSize;
};

END_RL_NS