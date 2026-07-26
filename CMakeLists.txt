# CMakeList.txt : CMake project for TestFramework, include source and define
# project specific logic here.
#
cmake_minimum_required(VERSION 3.30...4.4)
project("RLGymCPP" LANGUAGES CXX VERSION 0.0.1)

option(RLGYM_BUILD_API "Builds the API" OFF)
option(RLGYM_BUILD_ROCKET_LEAGUE "Builds the Rocket League implementation" OFF)

add_subdirectory("example")

if(RLGYM_BUILD_API)
	add_subdirectory("API")
endif()

if(RLGYM_BUILD_ROCKET_LEAGUE)
	add_subdirectory("RocketLeague")
endif()