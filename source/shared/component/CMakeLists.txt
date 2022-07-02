add_new_library(NAME component
    SOURCE_LIST
      source/Component.cpp
    INCLUDE_LIST
      include
    LINK_LIST
      log
      configuration
      cxxopts
      tcp_pubsub
      nlohmann_json
      error
    TEST_LIST
      test/Test.cpp
)

add_external_dependency(GITHUB_AUTHOR jarro2783 GITHUB_REPO cxxopts GITHUB_COMMIT v3.0.0
    TARGET_NAMES cxxopts
    OPTIONS "CXXOPTS_BUILD_TESTS OFF" "CXXOPTS_BUILD_EXAMPLES OFF")

# This fetches from my own fork since i need some modification (and Eclipse Foundation doesn't want me to do a PR on the official repo :D)
add_external_dependency(GITHUB_AUTHOR Slashcash GITHUB_REPO tcp_pubsub GITHUB_COMMIT cd7eab5eb31ece451b9d40880154fce14992ac9d
    TARGET_NAMES tcp_pubsub
    OPTIONS "TCP_PUBSUB_BUILD_SAMPLES OFF")

add_external_dependency(GITHUB_AUTHOR nlohmann GITHUB_REPO json GITHUB_COMMIT v3.10.5
    TARGET_NAMES nlohmann_json)