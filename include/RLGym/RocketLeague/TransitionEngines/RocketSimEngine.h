#pragma once

#include <RLGym/RocketLeague/Framework.h>
#include <RLGym/RocketLeague/RocketSim/GameState.h>
#include <RLGym/RocketLeague/RocketSim/typing.h>

#include <RLGym/API/typing.h>

#include <RocketSim/Sim/Arena/Arena.h>
#include <RocketSim/RocketSim.h>
#include <RocketSim/Sim/GameEventTracker/GameEventTracker.h>

#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
#endif

#include <array>
#include <ranges>
#include <filesystem>

using Eigen::VectorXf;

using namespace RLGYM_API_NS;
using namespace RLGYM_RL_NS::RGSim;
namespace fs = std::filesystem;

#define ROCKETSIM_ENGINE_ACTION RocketSimAction
#define ROCKETSIM_STATE(AgentIDType) GameState<AgentIDType>

START_RL_NS(TransitionEngines)

/// <summary>
/// A headless Rocket League TransitionEngine backed by RocketSim.
/// 
/// Simulates a normal soccar game with a single ball and any number of cars.
/// </summary>
/// <typeparam name="AgentID"></typeparam>
template<Hashable AgentID>
class RocketSimEngine : public TransitionEngine<AgentID, GameState<AgentID>, RocketSimAction> {
public:

	/// <summary>
	/// A headless Rocket League TransitionEngine backed by RocketSim.
	/// 
	/// Simulates a normal soccar game with a single ball and any number of cars.
	/// </summary>
	/// <param name="rlbotDelay">Enables RLBot-like 1 tick delay for actions. This forces the first action of the episode to no - op</param>
	/// <param name="gamemode">Allows you to select any other RocketSim supported GameMode</param>
	RocketSimEngine(bool rlbotDelay = true, RocketSim::GameMode gamemode = RocketSim::GameMode::SOCCAR) : m_rlbotDelay(rlbotDelay), m_mode(gamemode) {
		RocketSim::Init(fs::current_path() / "resources" / "RocketLeague" / "collision_meshes", true);
		this->m_arena = RocketSim::Arena::Create(gamemode);

		this->m_arena->SetGoalScoreCallback([](RocketSim::Arena* arena, RocketSim::Team team, void* userInfo) {
			static_cast<RocketSimEngine*>(userInfo)->_GoalScoreCallback(arena, team);
			}, this);

		this->m_arena->SetBallTouchCallback([](RocketSim::Arena* arena, RocketSim::Car* car, void* userInfo) {
			static_cast<RocketSimEngine*>(userInfo)->_BallTouchCallback(arena, car);
			}, this);
	};

	const GameState<AgentID>& GetState() override {
#ifdef TRACY_ENABLE
		ZoneScopedN("RocketSimEngine GetState");
#endif

		GameState<AgentID> gs = GameState<AgentID>();

		gs.tickCount = this->m_tickCount;
		gs.config = this->m_gameConfig.value();

		auto ballState = this->m_arena->ball->GetState();

		gs.ball = PhysicsObject();
		gs.ball.position = this->UnpackRocketSimVec(ballState.pos);
		gs.ball.linearVelocity = this->UnpackRocketSimVec(ballState.vel);
		gs.ball.angularVelocity = this->UnpackRocketSimVec(ballState.angVel);
		gs.ball.SetRotMat(this->UnpackRocketSimRotMat(ballState.rotMat));

		gs.goalScored = this->m_scoringTeam.has_value();
		if (this->m_scoringTeam.has_value()) {
			gs.scoringTeam = static_cast<int>(*this->m_scoringTeam);
		}

		gs.cars = {};

		for (auto& [agentID, rsimCar] : this->m_cars) {
			auto carState = rsimCar->GetState();

			Car car = Car<AgentID>();
			car.teamNum = static_cast<int>(rsimCar->team);
			car.hitboxType = this->m_hitboxes[rsimCar->id];

			car.physics = PhysicsObject();
			car.physics.position = this->UnpackRocketSimVec(carState.pos);
			car.physics.linearVelocity = this->UnpackRocketSimVec(carState.vel);
			car.physics.angularVelocity = this->UnpackRocketSimVec(carState.angVel);
			car.physics.SetRotMat(this->UnpackRocketSimRotMat(carState.rotMat));

			car.demoRespawnTimer = carState.demoRespawnTimer;
			std::copy(std::begin(carState.wheelsWithContact), std::end(carState.wheelsWithContact), car.wheelsWithContact.begin());
			car.supersonicTime = carState.supersonicTime;
			car.boostAmount = carState.boost;
			car.boostActiveTime = carState.timeSinceBoosted;
			car.handbrake = carState.handbrakeVal;

			car.hasJumped = carState.hasJumped;
			car.isHoldingJump = carState.lastControls.jump;
			car.isJumping = carState.isJumping;
			car.jumpTime = carState.jumpTime;

			car.hasFlipped = carState.hasFlipped;
			car.hasDoubleJumped = carState.hasDoubleJumped;
			car.airTimeSinceJump = carState.airTimeSinceJump;
			car.flipTime = carState.flipTime;

			auto flipTorque = this->UnpackRocketSimVec(carState.flipRelTorque);
			car.flipTorque = { flipTorque.x(), flipTorque.y() };

			car.isAutoflipping = carState.isAutoFlipping;
			car.autoflipTimer = carState.autoFlipTimer;
			car.autoflipDirection = carState.autoFlipTorqueScale;

			if (carState.carContact.cooldownTimer > 0) car.bumpVictimId = this->m_agentIDs[carState.carContact.otherCarID];
			else car.bumpVictimId = std::nullopt;

			car.ballTouches = this->m_touches[rsimCar->id];

			this->m_touches[rsimCar->id] = 0;
			gs.cars.emplace(agentID, car);
		}

		auto& boostPads = this->m_arena->GetBoostPads();
		gs.boostPads = std::vector<BoostPad>(boostPads.size());

		for (auto [idx, pad] : std::ranges::views::enumerate(boostPads)) {
			auto padState = pad->GetState();
			auto& padConfig = pad->config;

			BoostPad gsPad = {
				.isBigPad = padConfig.isBig,
				.cooldownTimer = padState.cooldown,
				.location = this->UnpackRocketSimVec(padConfig.pos),
			};

			gs.boostPads[idx] = gsPad;
		}

		this->m_state = gs;

		return this->m_state;
	}

	virtual GameState<AgentID> CreateBaseState() override {
		GameState<AgentID> gs = GameState<AgentID>();

		gs.tickCount = 0;
		gs.goalScored = false;

		gs.config = GameConfig();
		gs.config.gravity = 1;
		gs.config.boostConsumption = 1;
		gs.config.dodgeDeadzone = 0.5;

		gs.ball = PhysicsObject();
		gs.cars = {};
		gs.boostPads = std::vector<BoostPad>(static_cast<int>(this->m_arena->GetBoostPads().size()));

		return gs;
	};
	virtual const GameState<AgentID> Step(const AGENT_MAP(RocketSimAction)& actions, SharedInfo& sharedInfo) override {
#ifdef TRACY_ENABLE
		ZoneScopedNC("RocketSimEngine Step", tracy::Color::Blue3);
#endif

		int steps = 1;

		if (actions.size() != this->m_cars.size()) {
			throw new std::out_of_range(std::format("Expected actions for {} agents but received {}", this->m_cars.size(), actions.size()));
		}
		auto action = actions.begin();
		steps = action->second.size();

#ifdef TRACY_ENABLE
		ZoneNamedN(Arenastep, "Rocketsim Arena step", true); {
#endif
			for (int step = 0; step < steps; step++) {
				if (this->m_rlbotDelay) {
					this->m_arena->Step(1);
				}

				for (auto [agentID, action] : actions) {
					RocketSim::CarControls controls = {};

					controls.throttle = action[step][0];
					controls.steer = action[step][1];
					controls.pitch = action[step][2];
					controls.yaw = action[step][3];
					controls.roll = action[step][4];
					controls.jump = action[step][5];
					controls.boost = action[step][6];
					controls.handbrake = action[step][7];

					this->m_cars[agentID]->controls = controls;
				}

				if (!this->m_rlbotDelay) {
					this->m_arena->Step(1);
				}

				this->m_tickCount += 1;
			}
#ifdef TRACY_ENABLE
		};
#endif

		return this->GetState();
	};

	virtual const GameState<AgentID> SetState(const GameState<AgentID>& desiredState, SharedInfo& sharedInfo) override {
#ifdef TRACY_ENABLE
		ZoneScopedN("RocketSimEngine SetState");
#endif
		this->m_tickCount = desiredState.tickCount;

		RocketSim::MutatorConfig mutatorConfig = RocketSim::MutatorConfig(this->m_mode);
		mutatorConfig.gravity = RocketSim::Vec(
			0, 0, desiredState.config.gravity * -GRAVITY
		);
		mutatorConfig.boostUsedPerSecond = desiredState.config.boostConsumption * BOOST_CONSUMPTION_RATE;
		this->m_arena->SetMutatorConfig(mutatorConfig);
		this->m_gameConfig = desiredState.config;

		RocketSim::BallState ballState = RocketSim::BallState();
		ballState.pos = this->UnpackEigenVector3f(desiredState.ball.position);
		ballState.vel = this->UnpackEigenVector3f(desiredState.ball.linearVelocity);
		ballState.angVel = this->UnpackEigenVector3f(desiredState.ball.angularVelocity);

		ballState.rotMat = RocketSim::RotMat(
			this->UnpackEigenVector3f(desiredState.ball.Forward()),
			this->UnpackEigenVector3f(desiredState.ball.Right()),
			this->UnpackEigenVector3f(desiredState.ball.Up())
		);
		this->m_arena->ball->SetState(ballState);

		auto cars = this->m_arena->GetCars();

		for (RocketSim::Car* c : cars) {
			this->m_arena->RemoveCar(c);
		}

		this->m_cars.clear();
		this->m_agentIDs.clear();
		this->m_hitboxes.clear();
		this->m_touches.clear();
		this->m_scoringTeam = std::nullopt;

		for (auto& [agentID, desiredCar] : desiredState.cars) {
			RocketSim::CarConfig carConfig = RocketSim::CAR_CONFIG_OCTANE;


			switch (desiredCar.hitboxType) {
			case DOMINUS:
				carConfig = RocketSim::CAR_CONFIG_DOMINUS;
				break;
			case PLANK:
				carConfig = RocketSim::CAR_CONFIG_PLANK;
				break;
			case BREAKOUT:
				carConfig = RocketSim::CAR_CONFIG_BREAKOUT;
				break;
			case HYBRID:
				carConfig = RocketSim::CAR_CONFIG_HYBRID;
				break;
			case MERC:
				carConfig = RocketSim::CAR_CONFIG_MERC;
				break;

			}

			carConfig.dodgeDeadzone = desiredState.config.dodgeDeadzone;

			RocketSim::Car* car = this->m_arena->AddCar(
				static_cast<RocketSim::Team>(desiredCar.teamNum),
				carConfig
			);

			this->m_cars.emplace(agentID, car);
			this->m_agentIDs.emplace(car->id, agentID);
			this->m_hitboxes.emplace(car->id, desiredCar.hitboxType);
			this->m_touches.emplace(car->id, 0);
		}

		for (auto& [agentID, desiredCar] : desiredState.cars) {
			this->SetCarState(this->m_cars[agentID], desiredCar);
		}

		for (const auto& [idx, pad] : std::ranges::views::enumerate(this->m_arena->GetBoostPads())) {
			RocketSim::BoostPadState padState = pad->GetState();
			padState.cooldown = desiredState.boostPads[idx].cooldownTimer;
			pad->SetState(padState);
		}

		return this->GetState();
	};
	virtual void Close() {
	}; // Noop

	virtual const std::vector<AgentID> GetAgents() override {
		std::vector<AgentID> agents = {};

		for (const auto& [agent, car] : this->GetState().cars) {
			agents.push_back(agent);
		}

		return agents;
	};
	virtual const int GetMaxNumAgents() override { return 1337; };

	TRACY_ALLOC("RocketSim transition engine")

private:

	void _GoalScoreCallback(RocketSim::Arena* arena, RocketSim::Team scoringTeam) {
		this->m_scoringTeam = scoringTeam;
	}

	void _BallTouchCallback(RocketSim::Arena* arena, RocketSim::Car* car) {
		this->m_touches[car->id] += 1;
	}


	RocketSim::Vec UnpackEigenVector3f(Vector3f vec) {
		return RocketSim::Vec(
			vec.x(), vec.y(), vec.z()
		);
	}

	RocketSim::Vec UnpackEigenVector2f(Vector2f vec) {
		return RocketSim::Vec(
			vec.x(), vec.y(), 0
		);
	}

	Vector3f UnpackRocketSimVec(RocketSim::Vec vec) {
		return Vector3f(vec.x, vec.y, vec.z);
	}

	Matrix3f UnpackRocketSimRotMat(RocketSim::RotMat mat) {
		return Matrix3f({
				{mat[0][0], mat[0][1], mat[0][2]},
				{mat[1][0], mat[1][1], mat[1][2]},
				{mat[2][0], mat[2][1], mat[2][2]}
			});
	}

	void SetCarState(RocketSim::Car* car, Car<AgentID> desiredCar) {
		RocketSim::CarState carState = car->GetState();

		carState.pos = this->UnpackEigenVector3f(desiredCar.physics.position);
		carState.vel = this->UnpackEigenVector3f(desiredCar.physics.linearVelocity);
		carState.angVel = this->UnpackEigenVector3f(desiredCar.physics.angularVelocity);
		carState.rotMat = RocketSim::RotMat(
			this->UnpackEigenVector3f(desiredCar.physics.Forward()),
			this->UnpackEigenVector3f(desiredCar.physics.Right()),
			this->UnpackEigenVector3f(desiredCar.physics.Up())
		);

		carState.demoRespawnTimer = desiredCar.demoRespawnTimer;
		carState.isDemoed = desiredCar.IsDemoed();
		std::copy(desiredCar.wheelsWithContact.begin(), desiredCar.wheelsWithContact.end(), carState.wheelsWithContact);
		carState.isOnGround = desiredCar.OnGround();
		carState.supersonicTime = desiredCar.supersonicTime;
		carState.boost = desiredCar.boostAmount;
		carState.timeSinceBoosted = desiredCar.boostActiveTime;
		carState.handbrakeVal = desiredCar.handbrake;

		carState.hasJumped = desiredCar.hasJumped;
		carState.lastControls.jump = desiredCar.isHoldingJump;
		carState.isJumping = desiredCar.isJumping;
		carState.jumpTime = desiredCar.jumpTime;

		carState.hasFlipped = desiredCar.hasFlipped;
		carState.isFlipping = desiredCar.IsFlipping();
		carState.hasDoubleJumped = desiredCar.hasDoubleJumped;
		carState.airTimeSinceJump = desiredCar.airTimeSinceJump;
		carState.flipTime = desiredCar.flipTime;
		carState.flipRelTorque = this->UnpackEigenVector2f(desiredCar.flipTorque);

		carState.isAutoFlipping = desiredCar.isAutoflipping;
		carState.autoFlipTimer = desiredCar.autoflipTimer;
		carState.autoFlipTorqueScale = desiredCar.autoflipDirection;

		if (desiredCar.bumpVictimId.has_value()) {
			carState.carContact.otherCarID = this->m_cars.at(*desiredCar.bumpVictimId)->id;
		}

		car->SetState(carState);
	}

	bool m_rlbotDelay;
	GameState<AgentID> m_state;
	RocketSim::GameMode m_mode;
	RocketSim::Arena* m_arena;
	int m_tickCount = 0;
	std::optional<GameConfig> m_gameConfig = std::nullopt;
	AGENT_MAP(RocketSim::Car*) m_cars = {};
	std::unordered_map<int, AgentID> m_agentIDs = {};
	std::unordered_map<int, int> m_hitboxes = {};
	std::unordered_map<int, int> m_touches = {};
	std::optional<RocketSim::Team> m_scoringTeam = std::nullopt;
};

END_RL_NS