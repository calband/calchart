# The CalChart dependencies are managed by FetchContent, which means that
# cmake does all the heavy lifting to find and download the dependencies.
# When we need another package, add it to here.
# We use the convention of specifying the git tag completely, as that can
# avoid issues when a tag changes.

cmake_minimum_required(VERSION 3.11)

include(FetchContent)

# Options: prefer system deps for developer/CI builds, but allow forcing a full
# vendor (FetchContent) build for reproducible releases.
option(USE_SYSTEM_DEPENDENCIES "Prefer system-installed dependencies (find_package) when available" ON)
option(FORCE_VENDOR_DEPENDENCIES "Force building dependencies from source (using FetchContent). Useful for reproducible release builds." OFF)

# Helper macro: try to find a package with find_package; if not found and
# FORCE_VENDOR_DEPENDENCIES is ON then fetch via FetchContent (git repo + tag).
# Usage: try_find_or_fetch(<pkg_var_prefix> PACKAGE_NAME GIT_REPO GIT_TAG [EXTRA_BEFORE_FETCH...])
macro(try_find_or_fetch VAR_PREFIX PACKAGE_NAME GIT_REPO GIT_TAG)
  # Try to find a config/package first when allowed
  if(USE_SYSTEM_DEPENDENCIES)
    message(STATUS "Looking for system package ${PACKAGE_NAME}...")
    find_package(${PACKAGE_NAME} CONFIG QUIET)
  endif()

  # Detection: a config package typically provides a <Package>_FOUND or CMake targets
  set(_found FALSE)
  if(TARGET ${PACKAGE_NAME}::${PACKAGE_NAME})
    set(_found TRUE)
  elseif(DEFINED ${PACKAGE_NAME}_FOUND)
    if(${${PACKAGE_NAME}_FOUND})
      set(_found TRUE)
    endif()
  endif()

  if(_found)
    message(STATUS "Using system ${PACKAGE_NAME}")
    # Record discovery in the cache so other CMake scopes can see it and to
    # avoid PARENT_SCOPE warnings when this macro is called from top-level.
    set(${VAR_PREFIX}_FOUND TRUE CACHE INTERNAL "Found ${PACKAGE_NAME}")
  else()
    if(FORCE_VENDOR_DEPENDENCIES)
      message(STATUS "Fetching ${PACKAGE_NAME} from source (vendor mode): ${GIT_REPO} @ ${GIT_TAG}")
      FetchContent_Declare(
        ${VAR_PREFIX}
        GIT_REPOSITORY ${GIT_REPO}
        GIT_TAG ${GIT_TAG}
      )
      FetchContent_MakeAvailable(${VAR_PREFIX})
      set(${VAR_PREFIX}_FOUND TRUE CACHE INTERNAL "Fetched ${PACKAGE_NAME}")
    else()
      message(FATAL_ERROR "${PACKAGE_NAME} not found. Install it via your package manager (e.g. brew/apt/vcpkg) or re-run CMake with -DFORCE_VENDOR_DEPENDENCIES=ON to fetch sources.")
    endif()
  endif()
endmacro()

# ---- Per-dependency fetch content ----
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

FetchContent_Declare(
  wxUI
  GIT_REPOSITORY https://github.com/rmpowell77/wxUI.git
  GIT_TAG        aea5c52fb5d5525c2770954574acacaae24f9b3b # v0.2.2
)
FetchContent_MakeAvailable(wxUI)

# nlohmann_json (header-only). Prefer system (config package), fallback to fetch.
set(JSON_BuildTests OFF CACHE INTERNAL "")
try_find_or_fetch(nlohmann_json nlohmann_json https://github.com/nlohmann/json 55f93686c01528224f448c19128836e7df245f72)

try_find_or_fetch(Catch2 Catch2 https://github.com/catchorg/Catch2 6e79e68) # v3.4.0

if(USE_SYSTEM_DEPENDENCIES)
  find_package(wxWidgets REQUIRED COMPONENTS net core base gl aui html)
  include(${wxWidgets_USE_FILE})
elseif(FORCE_VENDOR_DEPENDENCIES)
  message(STATUS "Fetching wxWidgets sources for vendor build (this can be slow).")
  # Have wxWidgets build as static libraries
  set(wxBUILD_SHARED OFF)
  set(wxUSE_STL ON)
  set(wxUSE_STC OFF)
  set(wxUSE_STD_CONTAINERS ON)
  FetchContent_Declare(
    wxWidgets
    GIT_REPOSITORY "https://github.com/wxWidgets/wxWidgets.git"
    GIT_TAG 8aef5f40b93958719771331ca03866b7b6fff6bf # v3.2.8
  )
  FetchContent_MakeAvailable(wxWidgets)
else()
  message(FATAL_ERROR "wxWidgets not found. On macOS: brew install wxwidgets; On Ubuntu: apt install libwxgtk3.0-dev. Or re-run with -DFORCE_VENDOR_DEPENDENCIES=ON to fetch and build wxWidgets from source (release mode).")
endif()

# WebP format needed for wxWidgets 3.3.1
find_package(WebP CONFIG REQUIRED)

