#pragma once

#include <RLGym/RocketLeague/Framework.h>

#include <array>
#include <vector>

START_RL_NS(RGSim)

typedef std::array<float, 8> FloatArray8;
typedef std::vector<FloatArray8> RocketSimAction;

END_RL_NS