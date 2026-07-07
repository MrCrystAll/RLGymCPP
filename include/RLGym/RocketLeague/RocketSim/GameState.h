#pragma once

#include <RLGym/RocketLeague/Framework.h>
#include <RLGym/RocketLeague/RocketSim/Car.h>
#include <RLGym/RocketLeague/RocketSim/GameConfig.h>
#include <RLGym/RocketLeague/RocketSim/BoostPad.h>

#include <RLGym/API/typing.h>

#include <algorithm>
#include <optional>

START_RL_NS(RGSim)

template<typename AgentID>
class GameState {
public:
	int tickCount = 0;
	bool goalScored = false;
	std::optional<int> scoringTeam = std::nullopt;
	GameConfig config;
	AGENT_MAP(Car<AgentID>) cars = {};
	PhysicsObject ball = PhysicsObject();
	std::vector<BoostPad> boostPads = {};

	PhysicsObject InvertedBall() {
		if (this->m_invertedBall.has_value()) return this->m_invertedBall.value();

		this->m_invertedBall = this->ball.Inverted();
		return this->m_invertedBall.value();
	}

	std::vector<BoostPad> InvertedBoostPads() {
		if (this->m_invertedBoostPads.has_value()) return this->m_invertedBoostPads.value();

		this->m_invertedBoostPads = std::vector<BoostPad>(this->boostPads.size());
		std::reverse_copy(this->boostPads.begin(), this->boostPads.end(), this->m_invertedBoostPads->begin());
		return this->m_invertedBoostPads.value();
	}
private:
	std::optional<PhysicsObject> m_invertedBall = std::nullopt;
	std::optional<std::vector<BoostPad>> m_invertedBoostPads = std::nullopt;
};

END_RL_NS