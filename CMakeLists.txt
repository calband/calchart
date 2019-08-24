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
  "${CMAKE_CURRENT_SOURCE_DIR}/ccvers.h.in"
  "${PROJECT_BINARY_DIR}/version/ccvers.h"
  )


# calchart_core: A library of the non-UI parts of CalChart
# calchart_cmd: CLI for parsing files and testing
# CalChart: The App itself

# CalChart requires Boost.  Find it and include the directories here.
find_package (Boost 1.60 REQUIRED)

# CalChart requires BISON/FLEX for the grammar
find_package (BISON REQUIRED)
find_package (FLEX REQUIRED)

# calchart_core
set (target calchart_core)

BISON_TARGET (
  ${target}_parser
  "${CMAKE_CURRENT_SOURCE_DIR}/src/contgram.y"
  "${CMAKE_CURRENT_BINARY_DIR}/contgram.cpp"
  )

FLEX_TARGET (
  ${target}_lexer
  "${CMAKE_CURRENT_SOURCE_DIR}/src/contscan.l"
  "${CMAKE_CURRENT_BINARY_DIR}/contscan.cpp"
  COMPILE_FLAGS -i
  )

file (
  GLOB ${target}Sources "${CMAKE_CURRENT_SOURCE_DIR}/src/core/*.cpp"
  )

file (
  GLOB ${target}Headers "${CMAKE_CURRENT_SOURCE_DIR}/src/core/*.h" "${PROJECT_BINARY_DIR}/version/*.h"
  )

add_library (
  ${target}
  STATIC
  ${${target}Sources}
  ${${target}Headers}
  ${BISON_${target}_parser_OUTPUTS}
  ${FLEX_${target}_lexer_OUTPUTS}
  )

target_compile_definitions (${target} PUBLIC YY_NO_UNISTD_H)

set_target_properties (${target} PROPERTIES CXX_STANDARD 14)

target_include_directories (
  ${target}
  PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/src/core"
  "${CMAKE_CURRENT_SOURCE_DIR}/src"
  "${CMAKE_CURRENT_SOURCE_DIR}/resources"
  "${CMAKE_CURRENT_SOURCE_DIR}/submodules/munkres-cpp/src"
  "${Boost_INCLUDE_DIRS}"
  "${PROJECT_BINARY_DIR}/version"
  )

# calchart_cmd, Mac only
if (APPLE)
set (target calchart_cmd)
add_executable (
  ${target}
  "${CMAKE_CURRENT_SOURCE_DIR}/calchart_cmd/main.cpp"
  )

add_subdirectory (submodules/docopt.cpp EXCLUDE_FROM_ALL)

target_link_libraries (
  ${target}
  calchart_core
  docopt
  )

install (
  TARGETS ${target}
  DESTINATION bin
  COMPONENT tools
  )

set_target_properties (${target} PROPERTIES CXX_STANDARD 14)

target_include_directories (
  ${target}
  PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/src/core"
  "${CMAKE_CURRENT_SOURCE_DIR}/src"
  "${CMAKE_CURRENT_SOURCE_DIR}/resources"
  "${CMAKE_CURRENT_SOURCE_DIR}/submodules/munkres-cpp/src"
  "${Boost_INCLUDE_DIRS}"
  "${PROJECT_BINARY_DIR}/version"
  )

endif()

# CalChart
set (target CalChart)

file (
  GLOB ${target}Sources "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
  )

file (
  GLOB ${target}Headers "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
  )

file (
  GLOB ${target}Docs
    "${CMAKE_CURRENT_SOURCE_DIR}/docs/*"
  )

file (
  GLOB ${target}Resources
    "${CMAKE_CURRENT_SOURCE_DIR}/resources/*.xbm"
    "${CMAKE_CURRENT_SOURCE_DIR}/resources/*.icns"
  )

file (
  GLOB ${target}Images
    "${CMAKE_CURRENT_SOURCE_DIR}/resources/image/*"
  )

source_group(
  "Resources" FILES
  ${${target}Docs}
  ${${target}Resources}
  ${${target}Images}
  )

add_executable (
  ${target}
  ${${target}Sources}
  ${${target}Headers}
  ${${target}Docs}
  ${${target}Resources}
  ${${target}Images}
  "${CMAKE_CURRENT_SOURCE_DIR}/calchart.rc"
  )

install (
  TARGETS ${target}
  DESTINATION .
  COMPONENT applications
  )


set(AllResources ${${target}Docs} ${${target}Resources} ${${target}Images}) 

set_target_properties (
  ${target} PROPERTIES
  RESOURCE
  "${AllResources}"
  )

target_link_libraries (${target} calchart_core)

# We include wxWidgets as a submodule (see https://docs.wxwidgets.org/trunk/overview_cmake.html for info)
# Have wxWidgets build as static libraries
set (wxBUILD_SHARED OFF)
add_subdirectory (submodules/wxWidgets)

target_link_libraries (
  ${target} gl core base adv html net
  )

set_target_properties (${target} PROPERTIES CXX_STANDARD 14)

target_include_directories (
  ${target}
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
  ${target} PROPERTIES
  MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/build-macos/${target}-Info.plist.in"
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
  TARGET ${target} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${CMAKE_CURRENT_SOURCE_DIR}/resources/" $<TARGET_FILE_DIR:${target}>
  )

add_custom_command (
  TARGET ${target} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${CMAKE_CURRENT_SOURCE_DIR}/docs" $<TARGET_FILE_DIR:${target}>/docs
  )

endif ()

# we add some targets for docs and images so we can pack them for windows
# see http://yanivresearch.info/software/CPackWindowsTutorial/CPackForWindows.html
install (
  FILES ${${target}Docs}
  DESTINATION docs
  COMPONENT Docs
  )

install (
  FILES ${${target}Images}
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

