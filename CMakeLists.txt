# CMakeLists.txt

# some great sources to read to understand CMake:
# CMake Tutorial (https://cmake.org/cmake-tutorial/)
# CGold (https://cgold.readthedocs.io/en/latest/index.html)
# Effective Modern CMake (https://gist.github.com/mbinna/c61dbb39bca0e4fb7d1f73b0d66a4fd1)
# Creating Mac OS X Packages with CMake (https://blog.kitware.com/creating-mac-os-x-packages-with-cmake/)

cmake_minimum_required (VERSION 3.11)

project (CalChart)

set (CalChart_VERSION_MAJOR 3)
set (CalChart_VERSION_MINOR 5)
set (CalChart_VERSION_PATCH 4)

set (CalChart_VERSION "${CalChart_VERSION_MAJOR}.${CalChart_VERSION_MINOR}.${CalChart_VERSION_PATCH}")

configure_file (
  ${CMAKE_CURRENT_SOURCE_DIR}/ccvers.h.in
  ${PROJECT_BINARY_DIR}/version/ccvers.h
  )


# calchart_core: A library of the non-UI parts of CalChart
# calchart_cmd: CLI for parsing files and testing
# CalChart: The App itself

# CalChart requires Boost.  Find it and include the directories here.
find_package (Boost 1.60 REQUIRED)

# CalChart requires BISON/FLEX for the grammar
find_package (BISON REQUIRED)
find_package (FLEX REQUIRED)

add_subdirectory (submodules/munkres-cpp EXCLUDE_FROM_ALL)

# calchart_core
BISON_TARGET (
  calchart_core_parser
  ${CMAKE_CURRENT_SOURCE_DIR}/src/contgram.y
  ${CMAKE_CURRENT_BINARY_DIR}/contgram.cpp
  )

FLEX_TARGET (
  calchart_core_lexer
  ${CMAKE_CURRENT_SOURCE_DIR}/src/contscan.l
  ${CMAKE_CURRENT_BINARY_DIR}/contscan.cpp
  COMPILE_FLAGS -i
  )

add_library (
  calchart_core
  STATIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/animate.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/animate.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/animate_types.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/animatecommand.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/animatecommand.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/animatecompile.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/animatecompile.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/cc_continuity.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/cc_continuity.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/cc_coord.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/cc_coord.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/cc_drawcommand.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/cc_fileformat.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/cc_image.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/cc_image.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/cc_point.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/cc_point.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/cc_shapes.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/cc_shapes.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/cc_sheet.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/cc_sheet.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/cc_show.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/cc_show.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/cc_text.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/cc_text.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/cc_types.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/cont.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/cont.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/e7_transition_solver.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/e7_transition_solver.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/json.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/json.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/json_export.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/json_export.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/json_grammar.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/math_utils.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/math_utils.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/modes.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/modes.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/parse.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/print_ps.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/print_ps.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/prolog0.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/prolog1.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/prolog2.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/setup0.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/setup1.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/setup2.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/viewer_translate.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/viewer_translate.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/zllrbach.h
  ${PROJECT_BINARY_DIR}/version/ccvers.h
  ${BISON_calchart_core_parser_OUTPUTS}
  ${FLEX_calchart_core_lexer_OUTPUTS}
  )

target_compile_definitions (calchart_core PUBLIC YY_NO_UNISTD_H)

set_target_properties (calchart_core PROPERTIES CXX_STANDARD 14)

target_include_directories (
  calchart_core
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core
  ${Boost_INCLUDE_DIRS}
  ${PROJECT_BINARY_DIR}/version
  ${CMAKE_CURRENT_SOURCE_DIR}/submodules/munkres-cpp/src
  )

# calchart_cmd, Mac only
if (APPLE)
add_executable (
  calchart_cmd
  "${CMAKE_CURRENT_SOURCE_DIR}/calchart_cmd/main.cpp"
  )

add_subdirectory (submodules/docopt.cpp EXCLUDE_FROM_ALL)

target_link_libraries (
  calchart_cmd
  calchart_core
  docopt
  )

install (
  TARGETS calchart_cmd
  DESTINATION bin
  COMPONENT tools
  )

set_target_properties (calchart_cmd PROPERTIES CXX_STANDARD 14)

target_include_directories (
  calchart_cmd
  PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/src/core"
  "${CMAKE_CURRENT_SOURCE_DIR}/src"
  "${Boost_INCLUDE_DIRS}"
  "${PROJECT_BINARY_DIR}/version"
  )

endif()

# CalChart
file (
  GLOB CalChartSources "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
  )

file (
  GLOB CalChartHeaders "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
  )

file (
  GLOB CalChartDocs
    "${CMAKE_CURRENT_SOURCE_DIR}/docs/*"
  )

file (
  GLOB CalChartResources
    "${CMAKE_CURRENT_SOURCE_DIR}/resources/*.xbm"
    "${CMAKE_CURRENT_SOURCE_DIR}/resources/*.icns"
  )

file (
  GLOB CalChartImages
    "${CMAKE_CURRENT_SOURCE_DIR}/resources/image/*"
  )

source_group(
  "Resources" FILES
  ${CalChartDocs}
  ${CalChartResources}
  ${CalChartImages}
  )

add_executable (
  CalChart
  ${CalChartSources}
  ${CalChartHeaders}
  ${CalChartDocs}
  ${CalChartResources}
  ${CalChartImages}
  "${CMAKE_CURRENT_SOURCE_DIR}/calchart.rc"
  )

install (
  TARGETS CalChart
  DESTINATION .
  COMPONENT applications
  )


set(AllResources ${CalChartDocs} ${CalChartResources} ${CalChartImages}) 

set_target_properties (
  CalChart PROPERTIES
  RESOURCE
  "${AllResources}"
  )

target_link_libraries (CalChart calchart_core)

# We include wxWidgets as a submodule (see https://docs.wxwidgets.org/trunk/overview_cmake.html for info)
# Have wxWidgets build as static libraries
set (wxBUILD_SHARED OFF)
add_subdirectory (submodules/wxWidgets)

target_link_libraries (
  CalChart gl core base adv html net
  )

set_target_properties (CalChart PROPERTIES CXX_STANDARD 14)

target_include_directories (
  CalChart
  PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/src/core"
  "${CMAKE_CURRENT_SOURCE_DIR}/src"
  "${CMAKE_CURRENT_SOURCE_DIR}/resources"
  "${CMAKE_CURRENT_SOURCE_DIR}/submodules/munkres-cpp/src"
  "${Boost_INCLUDE_DIRS}"
  "${PROJECT_BINARY_DIR}/version"
  )

# Special stuff for Mac
if (APPLE)

set_target_properties (
  CalChart PROPERTIES
  MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/build-macos/CalChart-Info.plist.in"
  MACOSX_BUNDLE_ICON_FILE calchart.icns
  MACOSX_BUNDLE TRUE
  MACOSX_FRAMEWORK_IDENTIFIER com.calband.CalChart
  MACOSX_BUNDLE_GUI_IDENTIFIER com.calband.CalChart
  MACOSX_BUNDLE_BUNDLE_NAME CalChart
  MACOSX_BUNDLE_BUNDLE_VERSION ${CalChart_VERSION}
  MACOSX_BUNDLE_SHORT_VERSION_STRING ${CalChart_VERSION}
  MACOSX_BUNDLE_LONG_VERSION_STRING ${CalChart_VERSION}
  )

endif ()

# special stuff for Windows
if (MSVC)

set_property (DIRECTORY PROPERTY VS_STARTUP_PROJECT "CalChart")

set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")

add_custom_command (
  TARGET CalChart POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${CMAKE_CURRENT_SOURCE_DIR}/resources/" $<TARGET_FILE_DIR:CalChart>
  )

add_custom_command (
  TARGET CalChart POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${CMAKE_CURRENT_SOURCE_DIR}/docs" $<TARGET_FILE_DIR:CalChart>/docs
  )

endif ()

# we add some targets for docs and images so we can pack them for windows
# see http://yanivresearch.info/software/CPackWindowsTutorial/CPackForWindows.html
install (
  FILES ${CalChartDocs}
  DESTINATION docs
  COMPONENT Docs
  )

install (
  FILES ${CalChartImages}
  DESTINATION image
  COMPONENT Images
  )

# Installer section
# information about the DragNDrop installer was from:
# https://blog.kitware.com/creating-mac-os-x-packages-with-cmake/

include (InstallRequiredSystemLibraries)

set (CPACK_PACKAGE_NAME CalChart)
set (CPACK_ARCHIVE_COMPONENT_INSTALL ON)
if (MSVC)
  set (CPACK_COMPONENTS_ALL applications Docs Images)
else ()
  set (CPACK_COMPONENTS_ALL applications)
endif ()
set (CPACK_DMG_DS_STORE_SETUP_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/build-macos/CalChart_CMakeDMGSetup.scpt")
set (CPACK_DMG_BACKGROUND_IMAGE "${CMAKE_CURRENT_SOURCE_DIR}/build-macos/CalChart_DMGBackground.png")

set (CPACK_PACKAGE_VERSION_MAJOR "${CalChart_VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${CalChart_VERSION_MINOR}")
set (CPACK_PACKAGE_VERSION_PATCH "${CalChart_VERSION_PATCH}")
set (CPACK_PACKAGE_VERSION "${CalChart_VERSION}")

set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")

include (CPack)

# tests for calchart_core
include(CTest)
add_test (TutorialRuns calchart_cmd --version)


