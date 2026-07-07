#pragma once

#include <RLGym/RocketLeague/RocketSim/PhysicsObject.h>
#include <RLGym/RocketLeague/CommonValues.h>

START_RL_NS(RGSim)

struct BoostPad {
	bool isBigPad;
	float cooldownTimer;
	Vector3f location;
};

END_RL_NS