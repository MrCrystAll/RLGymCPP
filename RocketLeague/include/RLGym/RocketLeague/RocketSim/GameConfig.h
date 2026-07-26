#pragma once

#include <RLGym/RocketLeague/Framework.h>

START_RL_NS(RGSim)

class GameConfig {
public:
	float gravity = 0, boostConsumption = 0, dodgeDeadzone = 0;
};

END_RL_NS