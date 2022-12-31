# The CalChart dependencies are managed by FetchContent, which means that
# cmake does all the heavy lifting to find and download the dependencies.
# When we need another package, add it to here.
# We use the convention of specifying the git tag completely, as that can
# avoid issues when a tag changes.

cmake_minimum_required(VERSION 3.11)

include(FetchContent)

FetchContent_Declare(
  munkres-cpp
  GIT_REPOSITORY https://github.com/rmpowell77/munkres-cpp
  GIT_TAG        0f18953cb04f5a09fc8087f2086590481cc820b5
)
FetchContent_MakeAvailable(munkres-cpp)

FetchContent_Declare(
  docopt
  GIT_REPOSITORY "https://github.com/docopt/docopt.cpp"
  GIT_TAG 42ebcec9dc2c99a1b3a4542787572045763ad196 # v0.6.3
)
FetchContent_MakeAvailable(docopt)

set(JSON_BuildTests OFF CACHE INTERNAL "")
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY "https://github.com/nlohmann/json"
  GIT_TAG 4f8fba14066156b73f1189a2b8bd568bde5284c5 # v3.10.5
)
FetchContent_MakeAvailable(nlohmann_json)

# Have wxWidgets build as static libraries
set(wxBUILD_SHARED OFF)
set(wxUSE_STL ON)
set(wxUSE_STC OFF)
set(wxUSE_STD_CONTAINERS ON)
FetchContent_Declare(
  wxWidgets
  GIT_REPOSITORY "https://github.com/wxWidgets/wxWidgets"
  GIT_TAG a812fffda3fe686c94e24bff27e8effd96e4de64 # v3.2.2.1
)
FetchContent_MakeAvailable(wxWidgets)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2
  GIT_TAG        605a34765aa5d5ecbf476b4598a862ada971b0cc # v3.0.1
)
FetchContent_MakeAvailable(Catch2)

FetchContent_Declare(
  wxUI
  GIT_REPOSITORY https://github.com/rmpowell77/wxUI.git
  GIT_TAG        59e4da252d6d40445faf7f46d951d5bda190f46d # v0.1.3
)
FetchContent_MakeAvailable(wxUI)
