# Tests
# https://gitlab.com/CLIUtils/modern-cmake/-/blob/master/examples/extended-project/tests/CMakeLists.txt
cmake_minimum_required(VERSION 3.11)

add_executable(CalChartCoreTests
  ${CMAKE_CURRENT_SOURCE_DIR}/CalChartAnglesTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CalChartAnimationCommandTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CalChartAnimationSheetTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CalChartContinuityTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CalChartContinuityTokenTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CalChartCoordTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CalChartDiagnosticInfoTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CalChartDrawCommandTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CalChartDrawPrimativesTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CalChartFileFormatTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CalChartGitHubIssueSubmitterTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CalChartPointTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CalChartSheetTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CalChartShapesTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CalChartShowModeTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CalChartShowTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CalChartTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CalChartTextTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CalChartUtilsTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/PrintToPSTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/UpdateCheckerTests.cpp
)

SetupCompilerForTarget(CalChartCoreTests)

target_link_libraries(
  CalChartCoreTests
  PRIVATE
  calchart_core
  nlohmann_json::nlohmann_json
  Catch2::Catch2WithMain
)

# CTests section
if(NOT MSVC)
  add_test(NAME CalChartCoreTests COMMAND $<TARGET_FILE:CalChartCoreTests>)
endif()
