add_new_library(
  NAME
  log
  SOURCE_LIST
  ${CMAKE_CURRENT_SOURCE_DIR}/source/Logger.cpp
  LINK_LIST
  spdlog
  INCLUDE_LIST
  ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_external_dependency(
  GITHUB_AUTHOR
  gabime
  GITHUB_REPO
  spdlog
  GITHUB_COMMIT
  v1.9.2
  TARGET_NAMES
  spdlog)
