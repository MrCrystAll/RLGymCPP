cmake_minimum_required(VERSION 3.30...4.4)

add_executable(main "main.cpp")

# ====================== External dependencies (RLGym) ===========================

# You can download RLGym API (packaged) from the GitHub and place it wherever you want.
# Then, specify the path to where you extracted the API in this path
set(RLGymAPI_ROOT "ext/RLGym")
set(RLGymRL_ROOT "ext/RLGym")

set(RLGymAPI_DIR "${RLGymAPI_ROOT}/lib/cmake/RLGym/API")
set(RLGymRL_DIR "${RLGymRL_ROOT}/lib/cmake/RLGym/RL")

# ====================== External dependencies (RLGym Rocket League) ===========================

# RLGym Rocket League requires Eigen, json and RocketSim to compile, therefore you need to:

# Download Eigen (https://github.com/luxonis/eigen/releases/tag/3.4.0), cmake configure and cmake install Eigen.
# Then set this var to the path where you've put the installed Eigen.
set(Eigen_ROOT "ext/eigen")

# Download json (https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz), cmake configure and cmake install.
# Then set this var to the path where you've put the installed json.
set(json_ROOT "ext/json")

# Download RocketSim (https://github.com/MrCrystAll/RocketSim/releases/tag/v2.3.0) <- I already precompiled it for windows and linux there.
# Then set this var to the path where you've put the installed RocketSim.
set(RocketSim_ROOT "ext/RocketSim")

# ==== DO NOT TOUCH ====

set(Eigen3_DIR "${Eigen_ROOT}/share/eigen3/cmake")
set(nlohmann_json_DIR "${json_ROOT}/share/cmake/nlohmann_json")
set(RocketSim_DIR "${RocketSim_ROOT}/lib/cmake/RocketSim")

find_package(RLGymRL CONFIG REQUIRED)
target_link_libraries(main RLGym::RocketLeague)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET main PROPERTY CXX_STANDARD 26)
endif()

add_custom_command(TARGET main POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${RocketSim_BIN_DIR}/RocketSim.dll ${CMAKE_CURRENT_BINARY_DIR})
add_custom_command(TARGET main POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different "${CMAKE_CURRENT_SOURCE_DIR}/resources" ${CMAKE_CURRENT_BINARY_DIR}/resources)