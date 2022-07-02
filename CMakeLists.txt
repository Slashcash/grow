# Set minimum required cmake version
cmake_minimum_required(VERSION 3.20)
set(GROW_MINIMUM_CMAKE_VERSION ${CMAKE_MINIMUM_REQUIRED_VERSION})

# Set project name
project(
  grow
  VERSION 0.0.1
  LANGUAGES CXX)
set(GROW_PROJECT_VERSION ${PROJECT_VERSION})

# Enforcing c++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enforcing a static library build (not a good practice but we are having
# problems with relocatable code)
set(BUILD_SHARED_LIBS OFF)

# Adding some cmake utils
set(CMAKE_INCLUDE_FOLDER cmake)

include(${CMAKE_INCLUDE_FOLDER}/utils.cmake)

# We use CPM to manage dependencies (https://github.com/cpm-cmake/CPM.cmake)
find_program(GIT git REQUIRED)
set(CPM_DOWNLOAD_VERSION 0.34.0)
set(CPM_DOWNLOAD_LOCATION
    "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")

if(NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))
  message(STATUS "Downloading CPM.cmake to ${CPM_DOWNLOAD_LOCATION}")
  file(
    DOWNLOAD
    https://github.com/TheLartians/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
    ${CPM_DOWNLOAD_LOCATION})
endif()
include(${CPM_DOWNLOAD_LOCATION})

# We use clang-format to check and enforce code formatting
option(GROW_CODE_FORMAT_ENABLED "Enable code formatting" ON)

if(${GROW_CODE_FORMAT_ENABLED})
  find_program(CLANG_FORMAT clang-format REQUIRED)
  add_external_dependency(GITHUB_AUTHOR TheLartians GITHUB_REPO Format.cmake
                          GITHUB_COMMIT v1.7.3)
endif()

# We use clang-tidy for linting
option(GROW_LINTING_ENABLED "Code linting enabled" ON)
option(GROW_LINTING_WARNINGS_AS_ERRORS "Treats linting warnings as errors" ON)
set(CLANG_TIDY_FLAGS
    "*,\
    -llvm-header-guard,\
    -modernize-use-trailing-return-type,\
    -fuchsia-overloaded-operator,\
    -fuchsia-default-arguments-calls,\
    -cppcoreguidelines-pro-bounds-pointer-arithmetic,\
    -cppcoreguidelines-macro-usage")

if(${GROW_LINTING_ENABLED})
  find_program(CLANG_TIDY clang-tidy REQUIRED)
  set(CLANG_TIDY_CMD
      "${CLANG_TIDY};-header-filter=${CMAKE_SOURCE_DIR}/source/*;-checks=${CLANG_TIDY_FLAGS};"
  )

  if(${GROW_LINTING_WARNINGS_AS_ERRORS})
    set(CLANG_TIDY_CMD
        "${CLANG_TIDY_CMD}-warnings-as-errors=${CLANG_TIDY_FLAGS};")
  endif()

  set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY_CMD})
endif()

# We use google test for unit testing
option(GROW_BUILD_TESTS "Build automatic tests" ON)
if(${GROW_BUILD_TESTS})
  enable_testing()
endif()

# Adding shared utilities
set(SHARED_FOLDER source/shared)

add_subdirectory(${SHARED_FOLDER}/config)
add_subdirectory(${SHARED_FOLDER}/error)
add_subdirectory(${SHARED_FOLDER}/log)
add_subdirectory(${SHARED_FOLDER}/component)
add_subdirectory(${SHARED_FOLDER}/device)

# Components
set(COMPONENT_FOLDER source/component)

# Thermometer
option(GROW_BUILD_THERMOMETER "Build thermometer functionality" ON)
set(GROW_THERMOMETER_DEVICE "SimulatedThermometer")
if(${GROW_BUILD_THERMOMETER})
  list(FIND "SimulatedThermometer" ${GROW_THERMOMETER_DEVICE}
       VALID_THERMOMETER_DEVICE)
  if(NOT ${VALID_THERMOMETER_DEVICE})
    message(
      FATAL_ERROR "${GROW_THERMOMETER_DEVICE} is not a valid thermometer device"
    )
  endif()
  add_compile_definitions(THERMOMETER_DEVICE="${GROW_THERMOMETER_DEVICE}")
  add_subdirectory(${COMPONENT_FOLDER}/temperature)
endif()

# Providing some examples
set(EXAMPLES_FOLDER source/examples)

option(GROW_BUILD_EXAMPLES "Build code examples" ON)
if(${GROW_BUILD_EXAMPLES})
  add_subdirectory(${EXAMPLES_FOLDER}/component)
endif()

set(SCRIPTS_FOLDER "scripts")

# Automatically generating configuration file
set(CONFIG_GEN_PATH "${CMAKE_BINARY_DIR}")
set(CONFIG_GEN_OPTION -o ${CONFIG_GEN_PATH}/grow.cfg -v ${PROJECT_VERSION})

if(${GROW_BUILD_THERMOMETER})
  set(CONFIG_GEN_OPTION ${CONFIG_GEN_OPTION} -t)
endif()

add_custom_target(
  ConfigFile ALL
  COMMAND ${SCRIPTS_FOLDER}/generate_config.sh ${CONFIG_GEN_OPTION}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Generating configuration file")

install(FILES ${CONFIG_GEN_PATH}/grow.cfg DESTINATION etc/grow)
set(GROW_CONFIG_INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/etc/grow)

# Automatically generating dockerfile to build this project
set(BUILD_DOCKER_GEN_PATH "${CMAKE_BINARY_DIR}/Dockerfile_build")
set(BUILD_DOCKER_GEN_OPTION
    -o
    ${BUILD_DOCKER_GEN_PATH}
    -c
    ${GROW_MINIMUM_CMAKE_VERSION}
    -v
    ${GROW_PROJECT_VERSION}
    -d
    ${CMAKE_BINARY_DIR})

add_custom_target(
  DockerBuildFile
  COMMAND ${SCRIPTS_FOLDER}/generate_build_docker.sh ${BUILD_DOCKER_GEN_OPTION}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Generating build dockerfile")

# Automatically generating dockerfile to simulate this project
set(SIMULATION_DOCKER_GEN_PATH "${CMAKE_BINARY_DIR}/Dockerfile_simulation")
set(SIMULATION_DOCKER_GEN_OPTION
    -o
    ${SIMULATION_DOCKER_GEN_PATH}
    -c
    ${GROW_MINIMUM_CMAKE_VERSION}
    -v
    ${GROW_PROJECT_VERSION}
    -d
    ${CMAKE_BINARY_DIR})

add_custom_target(
  DockerSimulationFile
  COMMAND ${SCRIPTS_FOLDER}/generate_simulation_docker.sh ${BUILD_DOCKER_GEN_OPTION}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Generating simulation dockerfile")