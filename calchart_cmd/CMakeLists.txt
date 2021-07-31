# CMakeLists.txt

add_executable(
  calchart_cmd
  main.cpp
)

SetupCompilerForTarget(calchart_cmd)

target_include_directories(
  calchart_cmd
  PRIVATE
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

