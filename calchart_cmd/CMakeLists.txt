# CMakeLists.txt

add_executable (
  calchart_cmd
  main.cpp
  )

target_link_libraries (
  calchart_cmd
  calchart_core
  docopt
  nlohmann_json::nlohmann_json
  )

install (
  TARGETS calchart_cmd
  DESTINATION bin
  COMPONENT tools
  )

target_compile_options (calchart_cmd PRIVATE -Wall -Wextra)
set_target_properties (calchart_cmd PROPERTIES CXX_STANDARD 20)

target_include_directories (
  calchart_cmd
  PRIVATE
  "${PROJECT_BINARY_DIR}/version"
  )

