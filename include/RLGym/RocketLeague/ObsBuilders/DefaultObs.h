#pragma once

#include <RLGym/RocketLeague/RocketSim/GameState.h>
#include <RLGym/RocketLeague/RocketSim/typing.h>

#include <RLGym/API/typing.h>

#ifdef TRACY_ENABLE

#include <tracy/Tracy.hpp>

#endif

#include <string>
#include <optional>

using namespace RLGYM_API_NS;
using namespace RLGYM_RL_NS::RGSim;

START_RL_NS(ObsBuilders)

/// <summary>
/// The default observation builder.
/// </summary>
/// <typeparam name="AgentID">The type of the agent ID</typeparam>
template<Hashable AgentID>
class DefaultObs : public ObsBuilder<AgentID, std::vector<float>, GameState<AgentID>, std::tuple<std::string, int>> {
public:

	/// <summary>
	/// The default observation builder.
	/// </summary>
	/// <param name="zeroPadding">Number of max cars per team, if not std::nullopt the obs will be zero padded</param>
	/// <param name="positionCoefficient">Position normalization coefficient</param>
	/// <param name="linearVelocityCoefficient">Linear velocity normalization coefficient</param>
	/// <param name="angularVelocityCoefficient">Angular velocity normalization coefficient</param>
	/// <param name="padTimerCoefficient">Boost pad timers normalization coefficient</param>
	/// <param name="boostCoefficient">Player boost value normalization coefficient</param>
	DefaultObs(
		std::optional<int> zeroPadding = 3,
		Vector3f positionCoefficient = { 1 / 2300.0, 1 / 2300.0 , 1 / 2300.0 },
		Vector3f linearVelocityCoefficient = { 1 / 2300.0, 1 / 2300.0 , 1 / 2300.0 },
		Vector3f angularVelocityCoefficient = { 1 / EIGEN_PI, 1 / EIGEN_PI , 1 / EIGEN_PI },

		float padTimerCoefficient = 1 / 10.0,
		float boostCoefficient = 1 / 100.0
	) : m_zeroPadding(zeroPadding), m_positionCoefficient(positionCoefficient), m_linearVelocityCoefficient(linearVelocityCoefficient), m_angularVelocityCoefficient(angularVelocityCoefficient), m_padTimerCoefficient(padTimerCoefficient), m_boostCoefficient(boostCoefficient) {
	};

	const std::tuple<std::string, int> GetObservationSpace(const AgentID& agent) override {
		if (this->m_zeroPadding.has_value()) {
			return std::make_tuple(std::string("real"), this->SELF_PLUS_BALL_SIZE + this->PLAYER_PAD_SIZE * this->m_zeroPadding.value() * 2);
		}
		else if (this->m_state.has_value()) {
			return std::make_tuple(std::string("real"), this->SELF_PLUS_BALL_SIZE + this->PLAYER_PAD_SIZE * static_cast<int>(this->m_state->cars.size()));
		}
		return std::make_tuple(std::string("real"), -1);
	};
	const AGENT_MAP(std::vector<float>) BuildObs(const std::vector<AgentID>& agents, const GameState<AgentID>& state, SharedInfo& sharedInfo) override {
#ifdef TRACY_ENABLE
		ZoneScopedNC("Observation building", tracy::Color::Yellow);
#endif
		
		this->m_state = state;

		AGENT_MAP(std::vector<float>) observations = {};
		for (const AgentID& agent : agents) {
			std::vector<float> obs = {};

			const auto& [_, size] = this->GetObservationSpace(agent);

			obs.reserve(size);

			const Car<AgentID>& car = state.cars.at(agent);

			bool inverted = false;
			PhysicsObject ball = state.ball;
			std::vector<BoostPad> pads = state.boostPads;

			if (car.IsOrange()) {
				ball = state.InvertedBall();
				pads = state.InvertedBoostPads();
			}

			std::vector<float> boostTimers(pads.size());

			for (int i = 0; i < pads.size(); i++) {
				boostTimers[i] = pads[i].cooldownTimer * this->m_padTimerCoefficient;
			}

			obs.append_range(ball.position.cwiseProduct(this->m_positionCoefficient));
			obs.append_range(ball.linearVelocity.cwiseProduct(this->m_linearVelocityCoefficient));
			obs.append_range(ball.angularVelocity.cwiseProduct(this->m_angularVelocityCoefficient));
			obs.append_range(boostTimers);

			std::vector<float> miscData(
				{
					float(car.isHoldingJump),
					car.handbrake,
					float(car.hasJumped),
					float(car.isJumping),
					float(car.hasFlipped),
					float(car.IsFlipping()),
					float(car.hasDoubleJumped),
					float(car.CanFlip()),
					car.airTimeSinceJump
				}
			);

			obs.insert(obs.end(), miscData.begin(), miscData.end());

			auto carObs = this->GenerateCarObs(car, inverted);
			obs.append_range(carObs);

			std::vector<std::vector<float>> allies = {}, enemies = {};

			for (auto& [other, otherCar] : state.cars) {
				if (other == agent) continue;

				std::vector<std::vector<float>>& teamObs = allies;
				if (otherCar.teamNum != car.teamNum) teamObs = enemies;

				teamObs.push_back(this->GenerateCarObs(otherCar, inverted));
			}

			if (this->m_zeroPadding.has_value()) {
				while (allies.size() < this->m_zeroPadding.value() - 1) {
					allies.push_back(std::vector<float>(this->PLAYER_PAD_SIZE, 0));
				}
				while (enemies.size() < this->m_zeroPadding.value()) {
					enemies.push_back(std::vector<float>(this->PLAYER_PAD_SIZE, 0));
				}
			}

			for (auto& allyObs : allies) {
				obs.append_range(allyObs);
			}
			for (auto& enemyObs : enemies) {
				obs.append_range(enemyObs);
			}

			observations[agent] = obs;

		}

		return observations;
	};

	void Reset(const std::vector<AgentID>& agents, const GameState<AgentID>& initialState, SharedInfo& sharedInfo) override {
		this->m_state = initialState;
	};
	TRACY_ALLOC("Default observation builder")
protected:
	virtual std::vector<float> GenerateCarObs(
		const Car<AgentID>& car,
		bool inverted
	) {
		PhysicsObject physics = car.physics;
		if (inverted) physics = car.InvertedPhysics();

		std::vector<float> carObs = {};
		carObs.reserve(this->PLAYER_PAD_SIZE);

		carObs.append_range(physics.position.cwiseProduct(this->m_positionCoefficient));
		carObs.append_range(physics.Forward());
		carObs.append_range(physics.Up());
		carObs.append_range(physics.linearVelocity.cwiseProduct(this->m_linearVelocityCoefficient));
		carObs.append_range(physics.angularVelocity.cwiseProduct(this->m_angularVelocityCoefficient));

		std::vector<float> miscData = {
			car.boostAmount * this->m_boostCoefficient,
			car.demoRespawnTimer,
			float(car.OnGround()),
			float(car.IsBoosting()),
			float(car.IsSupersonic())
		};

		carObs.insert(carObs.end(), miscData.begin(), miscData.end());

		return carObs;
	}

private:
	std::optional<int> m_zeroPadding;

	Vector3f m_positionCoefficient, m_linearVelocityCoefficient, m_angularVelocityCoefficient;
	float m_boostCoefficient, m_padTimerCoefficient;

	const int PLAYER_PAD_SIZE = 20;
	const int SELF_PLUS_BALL_SIZE = 52;

	std::optional<GameState<AgentID>> m_state = std::nullopt;
};

END_RL_NS