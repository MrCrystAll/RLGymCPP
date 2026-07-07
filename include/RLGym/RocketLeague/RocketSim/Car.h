#pragma once

#include <RLGym/RocketLeague/RocketSim/PhysicsObject.h>
#include <RLGym/RocketLeague/CommonValues.h>

#include <Eigen/Dense>

#include <optional>
#include <tuple>


using Eigen::Vector2f;
using namespace RLGYM_RL_NS::CommonValues;

START_RL_NS(RGSim)

template<typename AgentID>
class Car {
public:
	int teamNum = -1, hitboxType = OCTANE, ballTouches = 0;
	std::optional<AgentID> bumpVictimId = std::nullopt;
	float demoRespawnTimer = 0.0, supersonicTime = 0.0, boostAmount = 0.0, boostActiveTime = 0.0, handbrake = 0.0, jumpTime = 0.0, airTimeSinceJump = 0.0, flipTime = 0.0, autoflipTimer = 0.0, autoflipDirection = 0.0;
    std::array<bool, 4> wheelsWithContact{};
	bool isJumping = false, hasJumped = false, isHoldingJump = false, hasFlipped = false, hasDoubleJumped = false, isAutoflipping = false;
	Vector2f flipTorque;
	PhysicsObject physics = PhysicsObject();

	const bool IsBlue() const {
		return this->teamNum == BLUE_TEAM;
	}

	const bool IsOrange() const {
		return this->teamNum == ORANGE_TEAM;
	}

    const bool IsDemoed() const {
        return demoRespawnTimer > 0.0f;
    }

    const bool IsBoosting() const {
        return boostActiveTime > 0.0f;
    }

    const bool IsSupersonic() const {
        return supersonicTime > 0.0f;
    }

    const bool OnGround() const {
        return std::count(wheelsWithContact.begin(),
            wheelsWithContact.end(),
            true) >= 3;
    }

    const bool HasFlip() const {
        return !hasDoubleJumped
            && !hasFlipped
            && airTimeSinceJump < DOUBLEJUMP_MAX_DELAY;
    }

    const bool CanFlip() const {
        return !OnGround()
            && !isHoldingJump
            && HasFlip();
    }

    const bool IsFlipping() const {
        return hasFlipped && flipTime < FLIP_TORQUE_TIME;
    }

    void SetIsFlipping(bool value) {
        if (value) {
            hasFlipped = true;
            if (flipTime >= FLIP_TORQUE_TIME)
                flipTime = 0.0f;
        }
        else {
            flipTime = FLIP_TORQUE_TIME;
        }
    }

    const bool HadCarContact() const {
        return bumpVictimId.has_value();
    }

    const PhysicsObject& InvertedPhysics() {
        if (!m_invertedPhysics.has_value())
            m_invertedPhysics = physics.Inverted();

        return *m_invertedPhysics;
    }

private:
	std::optional<PhysicsObject> m_invertedPhysics;

};

END_RL_NS