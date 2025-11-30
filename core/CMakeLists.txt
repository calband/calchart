# calchart_core

# CalChart requires BISON/FLEX for the grammar
find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)
find_package(CURL REQUIRED)

# calchart_core
BISON_TARGET(
  calchart_core_parser
  contgram.y
  ${CMAKE_CURRENT_BINARY_DIR}/contgram.cpp
)

FLEX_TARGET(
  calchart_core_lexer
  contscan.l
  ${CMAKE_CURRENT_BINARY_DIR}/contscan.cpp
  COMPILE_FLAGS -i
)


add_library(
  calchart_core
  STATIC
  CalChartAngles.h
  CalChartAnimation.cpp
  CalChartAnimation.h
  CalChartAnimationErrors.h
  CalChartAnimationSheet.cpp
  CalChartAnimationSheet.h
  CalChartAnimationCommand.cpp
  CalChartAnimationCommand.h
  CalChartAnimationCompile.cpp
  CalChartAnimationCompile.h
  CalChartAnimationTypes.h
  CalChartConstants.h
  CalChartConfiguration.cpp
  CalChartConfiguration.h
  CalChartContinuity.cpp
  CalChartContinuity.h
  CalChartContinuityToken.cpp
  CalChartContinuityToken.h
  CalChartCoord.h
  CalChartCoroutine.h
  CalChartDrawCommand.cpp
  CalChartDrawCommand.h
  CalChartDrawPrimatives.h
  CalChartFileFormat.h
  CalChartImage.cpp
  CalChartImage.h
  CalChartMeasure.h
  CalChartMovePointsTool.cpp
  CalChartMovePointsTool.h
  CalChartPoint.cpp
  CalChartPoint.h
  CalChartPostScript.h
  CalChartPrintContinuityLayout.cpp
  CalChartPrintContinuityLayout.h
  CalChartRanges.h
  CalChartSelectTool.cpp
  CalChartSelectTool.h
  CalChartShapes.cpp
  CalChartShapes.h
  CalChartSheet.cpp
  CalChartSheet.h
  CalChartShow.cpp
  CalChartShow.h
  CalChartShowMode.cpp
  CalChartShowMode.h
  CalChartPrintShowToPS.cpp
  CalChartPrintShowToPS.hpp
  CalChartText.cpp
  CalChartText.h
  CalChartTypes.h
  CalChartUtils.h
  e7_transition_solver.cpp
  e7_transition_solver.h
  linmath.h
  parse.h
  prolog0.h
  prolog2.h
  setup0.h
  setup2.h
  UpdateChecker.cpp
  UpdateChecker.h
  viewer_translate.cpp
  viewer_translate.h
  ${BISON_calchart_core_parser_OUTPUTS}
  ${FLEX_calchart_core_lexer_OUTPUTS}
)

target_compile_definitions(calchart_core PUBLIC YY_NO_UNISTD_H)
SetupCompilerForTarget(calchart_core)

# this is a hack to get contgram to build
target_include_directories(
  calchart_core
  PRIVATE
  ${PROJECT_SOURCE_DIR}/core
  INTERFACE
  ${PROJECT_SOURCE_DIR}/core
)

target_link_libraries(calchart_core PRIVATE
  nlohmann_json::nlohmann_json
  munkres
  CURL::libcurl
)

# When building on Windows with a static libcurl from vcpkg, define CURL_STATICLIB
# so curl headers avoid __declspec(dllimport) and we link statically.
if (WIN32)
  target_compile_definitions(calchart_core PRIVATE CURL_STATICLIB)
endif()

if(NOT MSVC)
  add_subdirectory(tests)
endif()
