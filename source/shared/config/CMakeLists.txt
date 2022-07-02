add_new_library(
  NAME
  configuration
  SOURCE_LIST
  source/Configuration.cpp
  LINK_LIST
  error
  config++
  TEST_LIST
  test/Test.cpp
  INCLUDE_LIST
  include)

add_external_dependency(
  GITHUB_AUTHOR
  hyperrealm
  GITHUB_REPO
  libconfig
  GITHUB_COMMIT
  v1.7.3
  TARGET_NAMES
  config++
  OPTIONS
  "BUILD_TESTS OFF"
  "BUILD_EXAMPLES OFF")

# libconfig++ cmake does not add include directories correctly :(, ugly
# workaround
get_target_property(LIBCONFIG_SOURCE_DIR config++ SOURCE_DIR)
target_include_directories(config++
                           PUBLIC "$<BUILD_INTERFACE:${LIBCONFIG_SOURCE_DIR}>")
