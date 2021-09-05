# Tests
# https://gitlab.com/CLIUtils/modern-cmake/-/blob/master/examples/extended-project/tests/CMakeLists.txt

add_executable(CalChartTests
  ${CMAKE_CURRENT_SOURCE_DIR}/CalChartContinuityTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CalChartContinuityTokenTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CalChartCoordTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CalChartFileFormatTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CalChartPointTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CalChartSheetTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CalChartShowTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CalChartTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CalChartTextTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/PrintToPSTests.cpp
)

SetupCompilerForTarget(CalChartTests)

target_include_directories(
  CalChartTests
  PRIVATE
  "${PROJECT_SOURCE_DIR}/submodules/Catch2/single_include"
  "${PROJECT_SOURCE_DIR}/src/core"
)

target_link_libraries(
  CalChartTests
  calchart_core
  nlohmann_json::nlohmann_json
)


