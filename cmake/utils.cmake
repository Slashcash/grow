function(add_external_dependency)
  set(oneValueArgs GITHUB_AUTHOR GITHUB_REPO GITHUB_COMMIT)
  set(multiValueArgs TARGET_NAMES OPTIONS)
  cmake_parse_arguments(DEPENDENCY "${options}" "${oneValueArgs}"
                        "${multiValueArgs}" ${ARGN})

  if(DEFINED DEPENDENCY_KEYWORDS_MISSING_VALUES)
    message(FATAL_ERROR "External dependency added with wrong arguments")
  endif()

  cpmaddpackage(
    NAME
    ${DEPENDENCY_GITHUB_REPO}
    EXCLUDE_FROM_ALL
    ON
    OPTIONS
    ${DEPENDENCY_OPTIONS}
    GIT_REPOSITORY
    https://github.com/${DEPENDENCY_GITHUB_AUTHOR}/${DEPENDENCY_GITHUB_REPO}
    GIT_TAG
    ${DEPENDENCY_GITHUB_COMMIT})

  # Manually disabling linting on external dependencies
  foreach(TARGET ${DEPENDENCY_TARGET_NAMES})
    set_target_properties(${TARGET} PROPERTIES CXX_CLANG_TIDY "")
  endforeach()
endfunction()

function(add_new_library)
  set(oneValueArgs NAME)
  set(multiValueArgs SOURCE_LIST INCLUDE_LIST LINK_LIST TEST_LIST)
  set(options HEADER_ONLY)
  cmake_parse_arguments(LIBRARY "${options}" "${oneValueArgs}"
                        "${multiValueArgs}" ${ARGN})

  if(DEFINED LIBRARY_KEYWORDS_MISSING_VALUES)
    message(FATAL_ERROR "New library added with wrong arguments")
  endif()

  # Here just because maybe we will want to append or prepend something
  set(FINAL_LIBRARY_NAME ${LIBRARY_NAME})

  if(${LIBRARY_HEADER_ONLY})
    add_library(${FINAL_LIBRARY_NAME} INTERFACE)
    target_sources(${FINAL_LIBRARY_NAME} INTERFACE ${LIBRARY_SOURCE_LIST})

    if(DEFINED LIBRARY_INCLUDE_LIST)
      target_include_directories(${FINAL_LIBRARY_NAME}
                                 INTERFACE ${LIBRARY_INCLUDE_LIST})
    endif()
  else()
    add_library(${FINAL_LIBRARY_NAME} ${LIBRARY_SOURCE_LIST})

    if(DEFINED LIBRARY_INCLUDE_LIST)
      target_include_directories(${FINAL_LIBRARY_NAME}
                                 PUBLIC ${LIBRARY_INCLUDE_LIST})
    endif()
  endif()

  if(DEFINED LIBRARY_LINK_LIST)
    if(${LIBRARY_HEADER_ONLY})
      target_link_libraries(${FINAL_LIBRARY_NAME}
                            INTERFACE ${LIBRARY_LINK_LIST})
    else()
      target_link_libraries(${FINAL_LIBRARY_NAME} PUBLIC ${LIBRARY_LINK_LIST})
    endif()
  endif()

  if(DEFINED LIBRARY_INTERFACE_LINK_LIST)
    target_link_libraries(${FINAL_LIBRARY_NAME}
                          INTERFACE ${LIBRARY_INTERFACE_LINK_LIST})
  endif()

  if(${GROW_BUILD_TESTS} AND DEFINED LIBRARY_TEST_LIST)
    set(TEST_NAME ${FINAL_LIBRARY_NAME}_test)
    add_external_dependency(
      GITHUB_AUTHOR
      google
      GITHUB_REPO
      googletest
      GITHUB_COMMIT
      release-1.11.0
      TARGET_NAMES
      gtest
      gtest_main)
    add_executable(${TEST_NAME} ${LIBRARY_TEST_LIST})

    # Disable linting on tests
    set_target_properties(${TEST_NAME} PROPERTIES CXX_CLANG_TIDY "")

    target_link_libraries(${TEST_NAME} gtest_main ${FINAL_LIBRARY_NAME})
    include(GoogleTest)
    gtest_discover_tests(${TEST_NAME})
  endif()
endfunction()

function(add_new_component)
  set(oneValueArgs NAME DESCRIPTION)
  set(multiValueArgs SOURCE_LIST INCLUDE_LIST LINK_LIST TEST_LIST)
  cmake_parse_arguments(COMPONENT "${options}" "${oneValueArgs}"
                        "${multiValueArgs}" ${ARGN})

  if(DEFINED COMPONENT_KEYWORDS_MISSING_VALUES)
    message(FATAL_ERROR "New component added with wrong arguments")
  endif()

  # Here just because maybe we will want to append or prepend something
  set(FINAL_COMPONENT_NAME ${COMPONENT_NAME})

  set(GENERATED_FOLDER ${CMAKE_CURRENT_BINARY_DIR}/generated)
  configure_file(${CMAKE_SOURCE_DIR}/cmake/ComponentHelper.hpp.in
                 ${GENERATED_FOLDER}/${COMPONENT_NAME}Base.hpp)
  configure_file(${CMAKE_SOURCE_DIR}/cmake/ComponentHelper.cpp.in
                 ${GENERATED_FOLDER}/main.cpp)

  add_executable(${FINAL_COMPONENT_NAME} ${COMPONENT_SOURCE_LIST}
                                         ${GENERATED_FOLDER}/main.cpp)

  if(DEFINED COMPONENT_INCLUDE_LIST)
    target_include_directories(
      ${FINAL_COMPONENT_NAME} PUBLIC ${COMPONENT_INCLUDE_LIST}
                                     ${GENERATED_FOLDER})
  endif()

  target_link_libraries(${FINAL_COMPONENT_NAME} PUBLIC component)
  if(DEFINED COMPONENT_LINK_LIST)
    target_link_libraries(${FINAL_COMPONENT_NAME} PUBLIC ${COMPONENT_LINK_LIST})
  endif()

  if(${GROW_BUILD_TESTS} AND DEFINED COMPONENT_TEST_LIST)
    set(TEST_NAME ${FINAL_COMPONENT_NAME}_test)
    add_external_dependency(
      GITHUB_AUTHOR
      google
      GITHUB_REPO
      googletest
      GITHUB_COMMIT
      release-1.11.0
      TARGET_NAMES
      gtest
      gtest_main)
    add_executable(${TEST_NAME} ${COMPONENT_TEST_LIST})

    # Disable linting on tests
    set_target_properties(${TEST_NAME} PROPERTIES CXX_CLANG_TIDY "")

    target_link_libraries(${TEST_NAME} gtest_main ${FINAL_LIBRARY_NAME})
    include(GoogleTest)
    gtest_discover_tests(${TEST_NAME})
  endif()

  install(TARGETS ${FINAL_COMPONENT_NAME} DESTINATION bin)
endfunction()
