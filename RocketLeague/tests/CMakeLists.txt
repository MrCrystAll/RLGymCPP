cmake_minimum_required(VERSION 3.30)

add_executable(RLGymTest "main.cpp")

target_link_libraries(RLGymTest PUBLIC RLGymRocketLeague)

if(ENABLE_TRACY)
	target_include_directories(RLGymTest PRIVATE ${tracy_SOURCE_DIR}/public)
	target_compile_definitions(RLGymTest PUBLIC TRACY_ENABLE)
	target_sources(RLGymTest PRIVATE ${tracy_SOURCE_DIR}/public/TracyClient.cpp)
endif()

add_custom_command(
	TARGET RLGymTest POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:RocketSim> $<TARGET_FILE_DIR:RLGymTest>
)

add_custom_command(
	TARGET RLGymTest POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different "${CMAKE_CURRENT_SOURCE_DIR}/resources/RocketLeague" $<TARGET_FILE_DIR:RLGymTest>/resources/RocketLeague
)

if(${CMAKE_BUILD_TYPE} STREQUAL Release)
	include(CheckIPOSupported)
	check_ipo_supported(RESULT supported OUTPUT error_message)

	if(supported)
		set_target_properties(RLGymTest PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
		message(STATUS "Using IPO/LTO for RLGymTest")
	else()
		message(WARNING "IPO/LTO not supported: ${error_message}")
	endif()
endif()


if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET RLGymTest PROPERTY CXX_STANDARD 26)
endif()