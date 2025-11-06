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

# Try to detect whether we're building from a git tag (release). If so, enable
# vendor dependencies automatically so releases are reproducible. This will
# enable FetchContent for dependencies unless the user explicitly overrides it.
execute_process(
  COMMAND git describe --tags --exact-match
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  RESULT_VARIABLE _git_tag_result
  OUTPUT_VARIABLE _git_tag
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
)
if(_git_tag_result EQUAL 0)
  message(STATUS "Git tag detected: ${_git_tag} — enabling vendor dependency fetch for a reproducible release build.")
  # If the user didn't explicitly request vendor deps, enable them for tags.
  # It's OK to set the cache here to make the behaviour visible in cmake-gui.
  if(NOT FORCE_VENDOR_DEPENDENCIES)
    set(FORCE_VENDOR_DEPENDENCIES ON CACHE BOOL "Force building dependencies from source for release builds" FORCE)
  endif()
endif()

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

# ---- Per-dependency find-or-fetch rules ----

# munkres-cpp (small utility): try system, otherwise fetch
try_find_or_fetch(munkres_cxx munkres-cpp https://github.com/rmpowell77/munkres-cpp 0f18953cb04f5a09fc8087f2086590481cc820b5)

# docopt (docopt.cpp) — try system package first
try_find_or_fetch(docopt docopt https://github.com/rmpowell77/docopt.cpp 692ba5b7061180298a3b3c290141029168e8c69b)

# nlohmann_json (header-only). Prefer system (config package), fallback to fetch.
set(JSON_BuildTests OFF CACHE INTERNAL "")
try_find_or_fetch(nlohmann_json nlohmann_json https://github.com/nlohmann/json 55f93686c01528224f448c19128836e7df245f72)

# wxWidgets — special handling: prefer system wxWidgets, otherwise fetch and
# attempt a vendor build with the same options used before.
if(USE_SYSTEM_DEPENDENCIES)
  message(STATUS "Trying to find system wxWidgets...")
  # Request the components the project needs so imported targets like wx::core
  # and wx::net are created by the find_package call.
  find_package(wxWidgets COMPONENTS core base net gl xml adv QUIET)
endif()

if(wxWidgets_FOUND OR TARGET wx::core)
  # If the find_package produced imported targets (wx::core, wx::net, wx::gl etc)
  # then we can use them directly. Otherwise, the system wxWidgets may only
  # provide variables (wxWidgets_LIBRARIES) and not the modern imported targets
  # this project expects. Detect that case and fall back to vendor build if
  # allowed.
  set(_wx_targets_ok TRUE)
  foreach(_comp core net gl)
    if(NOT TARGET wx::${_comp})
      set(_wx_targets_ok FALSE)
    endif()
  endforeach()
  if(_wx_targets_ok)
    message(STATUS "Using system wxWidgets ${wxWidgets_VERSION_STRING} (imported targets available)")
    set(wxwidgets_found TRUE)
  else()
    if(FORCE_VENDOR_DEPENDENCIES)
      message(WARNING "System wxWidgets found but does not provide modern imported targets (wx::core, wx::net, wx::gl). Falling back to vendor build.")
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
      set(wxwidgets_found TRUE)
    else()
      # Create lightweight compatibility imported targets from the legacy
      # FindwxWidgets variables so developer builds using brew/pkg can work
      # without requiring a full wxWidgets vendor build. These targets are
      # pragmatic shims and may not capture every transitive usage requirement
      # of an upstream CMake config, but are sufficient for most builds.
      message(WARNING "System wxWidgets found but does not provide modern imported targets (wx::core, wx::net, wx::gl). Creating compatibility imported targets from legacy variables.")

      if(NOT TARGET wx::core)
        add_library(wx::core UNKNOWN IMPORTED)
        set_target_properties(wx::core PROPERTIES
          INTERFACE_INCLUDE_DIRECTORIES "${wxWidgets_INCLUDE_DIRS}"
          INTERFACE_LINK_LIBRARIES "${wxWidgets_LIBRARIES}"
        )
        if(DEFINED wxWidgets_COMPILE_DEFINITIONS)
          target_compile_definitions(wx::core INTERFACE ${wxWidgets_COMPILE_DEFINITIONS})
        endif()
      endif()

      if(NOT TARGET wx::net)
        add_library(wx::net UNKNOWN IMPORTED)
        set_target_properties(wx::net PROPERTIES
          INTERFACE_INCLUDE_DIRECTORIES "${wxWidgets_INCLUDE_DIRS}"
          INTERFACE_LINK_LIBRARIES "${wxWidgets_LIBRARIES}"
        )
        if(DEFINED wxWidgets_COMPILE_DEFINITIONS)
          target_compile_definitions(wx::net INTERFACE ${wxWidgets_COMPILE_DEFINITIONS})
        endif()
      endif()

      if(NOT TARGET wx::gl)
        add_library(wx::gl UNKNOWN IMPORTED)
        set_target_properties(wx::gl PROPERTIES
          INTERFACE_INCLUDE_DIRECTORIES "${wxWidgets_INCLUDE_DIRS}"
          INTERFACE_LINK_LIBRARIES "${wxWidgets_LIBRARIES}"
        )
        if(DEFINED wxWidgets_COMPILE_DEFINITIONS)
          target_compile_definitions(wx::gl INTERFACE ${wxWidgets_COMPILE_DEFINITIONS})
        endif()
      endif()

      set(wxwidgets_found TRUE)
    endif()
  endif()
else()
  if(FORCE_VENDOR_DEPENDENCIES)
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
    set(wxwidgets_found TRUE)
  else()
    message(FATAL_ERROR "wxWidgets not found. On macOS: brew install wxwidgets; On Ubuntu: apt install libwxgtk3.0-dev. Or re-run with -DFORCE_VENDOR_DEPENDENCIES=ON to fetch and build wxWidgets from source (release mode).")
  endif()
endif()

# Catch2 (tests) — try to find system package, otherwise fetch
try_find_or_fetch(Catch2 Catch2 https://github.com/catchorg/Catch2 6e79e68)

# wxUI (project-specific helper) — prefer vendored fetch if not available
try_find_or_fetch(wxUI wxUI https://github.com/rmpowell77/wxUI.git aea5c52fb5d5525c2770954574acacaae24f9b3b)
