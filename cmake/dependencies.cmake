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
  GIT_REPOSITORY "https://github.com/rmpowell77/docopt.cpp"
  GIT_TAG 692ba5b7061180298a3b3c290141029168e8c69b # fix for cmake
)
FetchContent_MakeAvailable(docopt)

set(JSON_BuildTests OFF CACHE INTERNAL "")
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY "https://github.com/nlohmann/json"
  GIT_TAG 55f93686c01528224f448c19128836e7df245f72 # v3.12.0
)
FetchContent_MakeAvailable(nlohmann_json)

# Have wxWidgets build as static libraries
set(wxBUILD_SHARED OFF)
set(wxUSE_STL ON)
set(wxUSE_STC OFF)
set(wxUSE_STD_CONTAINERS ON)
set(wxUSE_STD_STRING_CONV_IN_WXSTRING ON)
FetchContent_Declare(
  wxWidgets
  GIT_REPOSITORY "https://github.com/wxWidgets/wxWidgets.git"
  GIT_TAG 49c6810948f40c457e3d0848b9111627b5b61de5 # v3.3.1
)
FetchContent_MakeAvailable(wxWidgets)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2
  GIT_TAG        6e79e68 # v3.4.0
)
FetchContent_MakeAvailable(Catch2)

FetchContent_Declare(
  wxUI
  GIT_REPOSITORY https://github.com/rmpowell77/wxUI.git
  GIT_TAG        aea5c52fb5d5525c2770954574acacaae24f9b3b # v0.2.2
)
FetchContent_MakeAvailable(wxUI)
