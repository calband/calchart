# CMakeLists.txt

add_executable(
  calchart_cmd
  calchart_cmd_parse_continuity_text.hpp
  calchart_cmd_parse.hpp
  main.cpp
)

SetupCompilerForTarget(calchart_cmd)

target_include_directories(
  calchart_cmd
  PRIVATE
  ${PROJECT_SOURCE_DIR}/core
)

target_link_libraries(
  calchart_cmd
  calchart_core
  docopt
  nlohmann_json::nlohmann_json
)

install(
  TARGETS calchart_cmd
  DESTINATION bin
  COMPONENT tools
)

