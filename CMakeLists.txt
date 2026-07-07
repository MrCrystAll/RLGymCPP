# CMakeList.txt : CMake project for TestFramework, include source and define
# project specific logic here.
#
cmake_minimum_required(VERSION 3.30...4.4)
project("RLGymCPP" LANGUAGES CXX)

# Add source to this project's executable.
add_library (RLGymAPI INTERFACE "include/RLGym/API/Framework.h" "include/RLGym/API/RLGym.h" "include/RLGym/API/typing.h")
target_include_directories(RLGymAPI INTERFACE "include")


add_subdirectory("libsrc/eigen")
# Add source to this project's executable.
add_library (RLGymRocketLeague STATIC 
	"include/RLGym/RocketLeague/CommonValues.h"
	"include/RLGym/RocketLeague/Framework.h"
	"include/RLGym/RocketLeague/RocketLeague.h"

	"include/RLGym/RocketLeague/TransitionEngines/RocketSimEngine.h"
	"include/RLGym/RocketLeague/TransitionEngines/all.h"

	"include/RLGym/RocketLeague/RocketSim/PhysicsObject.h" 
	"include/RLGym/RocketLeague/RocketSim/Math.h" 
	"include/RLGym/RocketLeague/RocketSim/Car.h"
	"include/RLGym/RocketLeague/RocketSim/GameState.h"
	"include/RLGym/RocketLeague/RocketSim/GameConfig.h"
	"include/RLGym/RocketLeague/RocketSim/typing.h"
	"include/RLGym/RocketLeague/RocketSim/BoostPad.h"
	"include/RLGym/RocketLeague/RocketSim/all.h"

	"include/RLGym/RocketLeague/ActionParsers/RepeatAction.h"
	"include/RLGym/RocketLeague/ActionParsers/LookupTableAction.h"
	"include/RLGym/RocketLeague/ActionParsers/all.h"

	"include/RLGym/RocketLeague/ObsBuilders/DefaultObs.h"
	"include/RLGym/RocketLeague/ObsBuilders/all.h"

	"include/RLGym/RocketLeague/RewardFunctions/TouchReward.h"
	"include/RLGym/RocketLeague/RewardFunctions/all.h"

	"include/RLGym/RocketLeague/DoneConditions/GoalCondition.h"
	"include/RLGym/RocketLeague/DoneConditions/all.h"

	"include/RLGym/RocketLeague/StateMutators/FixedTeamSizeMutator.h"
	"include/RLGym/RocketLeague/StateMutators/MutatorSequence.h"
	"include/RLGym/RocketLeague/StateMutators/KickoffMutator.h"
	"include/RLGym/RocketLeague/StateMutators/all.h"

	"include/RLGym/RocketLeague/Renderers/RocketSimVisRenderer.h"
	"include/RLGym/RocketLeague/Renderers/all.h"
 "test.cpp" "include/RLGym/RocketLeague/DoneConditions/TimeoutCondition.h" "include/RLGym/RocketLeague/DoneConditions/AnyCondition.h")

target_include_directories(RLGymRocketLeague PUBLIC "include" "libsrc/json/include"
)

target_link_libraries(RLGymRocketLeague PUBLIC RLGymAPI Eigen3::Eigen RocketSim)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET RLGymAPI PROPERTY CXX_STANDARD 26)
  set_property(TARGET RLGymRocketLeague PROPERTY CXX_STANDARD 26)
endif()

# TODO: Add tests and install targets if needed.
