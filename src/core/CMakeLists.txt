# calchart_core

# CalChart requires BISON/FLEX for the grammar
find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

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
  CalChartContinuity.cpp
  CalChartContinuity.h
  CalChartContinuityToken.cpp
  CalChartContinuityToken.h
  CalChartCoord.h
  CalChartDrawCommand.cpp
  CalChartDrawCommand.h
  CalChartDrawPrimatives.h
  CalChartFileFormat.h
  CalChartImage.cpp
  CalChartImage.h
  CalChartMovePointsTool.cpp
  CalChartMovePointsTool.h
  CalChartPoint.cpp
  CalChartPoint.h
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
  CalChartText.cpp
  CalChartText.h
  CalChartTypes.h
  CalChartUtils.h
  e7_transition_solver.cpp
  e7_transition_solver.h
  linmath.h
  parse.h
  print_ps.cpp
  print_ps.h
  prolog0.h
  prolog2.h
  setup0.h
  setup2.h
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
  ${PROJECT_SOURCE_DIR}/src/core
  INTERFACE
  ${PROJECT_SOURCE_DIR}/src/core
)

target_link_libraries(calchart_core PRIVATE
  nlohmann_json::nlohmann_json
  munkres
)
 

