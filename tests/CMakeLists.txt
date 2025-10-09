# Tests
# https://gitlab.com/CLIUtils/modern-cmake/-/blob/master/examples/extended-project/tests/CMakeLists.txt
cmake_minimum_required(VERSION 3.11)

include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2
  GIT_TAG        605a34765aa5d5ecbf476b4598a862ada971b0cc # v3.0.1
)
FetchContent_MakeAvailable(Catch2)

add_executable(CalChartTests
  ${CMAKE_CURRENT_SOURCE_DIR}/CalChartAnglesTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CalChartAnimationCommandTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CalChartAnimationSheetTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CalChartContinuityTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CalChartContinuityTokenTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CalChartCoordTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CalChartDrawCommandTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CalChartDrawPrimativesTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CalChartFileFormatTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CalChartPointTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CalChartSheetTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CalChartShapesTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CalChartShowModeTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CalChartShowTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CalChartTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CalChartTextTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CalChartUtilsTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/DCSaveRestoreTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/PrintToPSTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/UpdateCheckerTests.cpp
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
  wx::core
)


