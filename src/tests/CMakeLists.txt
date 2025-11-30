# Tests
# https://gitlab.com/CLIUtils/modern-cmake/-/blob/master/examples/extended-project/tests/CMakeLists.txt
cmake_minimum_required(VERSION 3.11)

add_executable(CalChartTests
  ${CMAKE_CURRENT_SOURCE_DIR}/DCSaveRestoreTests.cpp
)

SetupCompilerForTarget(CalChartTests)

target_include_directories(
  CalChartTests
  PRIVATE
  "${PROJECT_SOURCE_DIR}/src"
)

target_link_libraries(
  CalChartTests
  PRIVATE
  calchart_core
  nlohmann_json::nlohmann_json
  Catch2::Catch2WithMain
  ${wxWidgets_LIBRARIES}
)

# CTests section
if(NOT MSVC)
  add_test(NAME CalChartTests COMMAND $<TARGET_FILE:CalChartTests>)
endif()

