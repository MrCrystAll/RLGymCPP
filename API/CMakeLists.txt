# CMakeList.txt : CMake project for TestFramework, include source and define
# project specific logic here.
#
cmake_minimum_required(VERSION 3.30...4.4)
project("RLGymAPI" LANGUAGES CXX VERSION 0.0.1)

option(RLGYM_INSTALL_API "Installs RLGym" ${PROJECT_IS_TOP_LEVEL})

# Add source to this project's executable.
add_library (${PROJECT_NAME} INTERFACE "include/RLGym/API/Framework.h" "include/RLGym/API/RLGym.h" "include/RLGym/API/typing.h" "include/RLGym/api.h")
target_include_directories(${PROJECT_NAME} INTERFACE 
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 26)
endif()

if(RLGYM_INSTALL_API)
	include(CMakePackageConfigHelpers)

	write_basic_package_version_file(
		"${PROJECT_BINARY_DIR}/RLGymAPIConfigVersion.cmake"
		VERSION ${PROJECT_VERSION}
		COMPATIBILITY AnyNewerVersion
	)

	install(TARGETS ${PROJECT_NAME}
		EXPORT RLGymAPITargets
		LIBRARY DESTINATION lib COMPONENT Runtime
		ARCHIVE DESTINATION lib COMPONENT Development
		RUNTIME DESTINATION bin COMPONENT Runtime
		PUBLIC_HEADER DESTINATION include COMPONENT Development
		BUNDLE DESTINATION bin COMPONENT Runtime
	)

	configure_package_config_file(
		"${PROJECT_SOURCE_DIR}/cmake/RLGymAPIConfig.cmake.in"
		"${PROJECT_BINARY_DIR}/RLGymAPIConfig.cmake"
		INSTALL_DESTINATION lib/cmake/RLGym/API
	)

	install(EXPORT RLGymAPITargets DESTINATION lib/cmake/RLGym/API)
	install(FILES "${PROJECT_BINARY_DIR}/RLGymAPIConfigVersion.cmake"
				  "${PROJECT_BINARY_DIR}/RLGymAPIConfig.cmake"
			DESTINATION lib/cmake/RLGym/API)
	install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/RLGym DESTINATION include)
endif()