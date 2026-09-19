cmake_minimum_required(VERSION 3.30...4.4)
project("RLGymRocketLeague" LANGUAGES CXX VERSION 0.0.1)

include(GNUInstallDirs)

option(RLGYM_INSTALL_RL "Installs RLGym Rocket League" ${RLGYM_STANDALONE})

if(NOT TARGET Eigen3::Eigen)
	set(Eigen3_DIR ${CMAKE_SOURCE_DIR}/ext/eigen/share/eigen3/cmake)
	find_package(Eigen3 CONFIG REQUIRED)
endif()

if(NOT TARGET nlohmann_json)
	set(nlohmann_json_DIR ${CMAKE_SOURCE_DIR}/ext/json/share/cmake/nlohmann_json)
	find_package(nlohmann_json CONFIG REQUIRED)
endif()

if(NOT TARGET RocketSim)
	set(RocketSim_DIR ${CMAKE_SOURCE_DIR}/ext/RocketSim/lib/cmake/RocketSim)
	find_package(RocketSim CONFIG REQUIRED)
endif()

# Add source to this project's executable.
add_library (RLGymRocketLeague INTERFACE 
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
	"include/RLGym/RocketLeague/DoneConditions/TimeoutCondition.h"
	"include/RLGym/RocketLeague/DoneConditions/AnyCondition.h"
	"include/RLGym/RocketLeague/DoneConditions/all.h"

	"include/RLGym/RocketLeague/StateMutators/FixedTeamSizeMutator.h"
	"include/RLGym/RocketLeague/StateMutators/MutatorSequence.h"
	"include/RLGym/RocketLeague/StateMutators/KickoffMutator.h"
	"include/RLGym/RocketLeague/StateMutators/all.h"

	"include/RLGym/RocketLeague/Renderers/RocketSimVisRenderer.h"
	"include/RLGym/RocketLeague/Renderers/all.h")

target_include_directories(${PROJECT_NAME} INTERFACE 
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(${PROJECT_NAME} INTERFACE
    RLGymAPI
    Eigen3::Eigen
    RocketSim::RocketSim
    nlohmann_json::nlohmann_json
)

if(ENABLE_TRACY)
	target_include_directories(RLGymRocketLeague INTERFACE ${tracy_SOURCE_DIR}/public)
	target_compile_definitions(RLGymRocketLeague INTERFACE TRACY_ENABLE)
	target_sources(RLGymRocketLeague PRIVATE ${tracy_SOURCE_DIR}/public/TracyClient.cpp)
endif()

if (CMAKE_VERSION VERSION_GREATER 3.12)
	set_property(TARGET RLGymRocketLeague PROPERTY CXX_STANDARD 26)
endif()

set_target_properties(RLGymRocketLeague PROPERTIES
    EXPORT_NAME RocketLeague
)

if(RLGYM_INSTALL_RL)
	include(CMakePackageConfigHelpers)

	set(ROCKETSIM_INSTALL OFF)

	write_basic_package_version_file(
		"${PROJECT_BINARY_DIR}/RLGymRLConfigVersion.cmake"
		VERSION ${PROJECT_VERSION}
		COMPATIBILITY AnyNewerVersion
	)

	install(TARGETS RLGymRocketLeague
		EXPORT RLGymRLTargets
		INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
	)

	install(EXPORT RLGymRLTargets
		FILE RLGymRLTargets.cmake
		NAMESPACE RLGym::
		DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/RLGym/RL
	)


	configure_package_config_file(
		"${PROJECT_SOURCE_DIR}/cmake/RLGymRLConfig.cmake.in"
		"${PROJECT_BINARY_DIR}/RLGymRLConfig.cmake"
		INSTALL_DESTINATION lib/cmake/RLGym/RL
	)

	install(FILES "${PROJECT_BINARY_DIR}/RLGymRLConfigVersion.cmake"
				  "${PROJECT_BINARY_DIR}/RLGymRLConfig.cmake"
			DESTINATION lib/cmake/RLGym/RL)
	install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/RLGym DESTINATION include)
endif()