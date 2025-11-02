# CMakeLists.txt

# some great sources to read to understand CMake:
# CMake Tutorial (https://cmake.org/cmake-tutorial/)
# CGold (https://cgold.readthedocs.io/en/latest/index.html)
# Effective Modern CMake (https://gist.github.com/mbinna/c61dbb39bca0e4fb7d1f73b0d66a4fd1)
# Creating Mac OS X Packages with CMake (https://blog.kitware.com/creating-mac-os-x-packages-with-cmake/)
#
# How to add files:
# We've structured CMake so that it will include the subdirectories needed.
# That means that when you want to add a new file, you'll have to modify the
# cmake file in that directory.

cmake_minimum_required(VERSION 3.11)

project(CalChart)

# prevent In-source builds:
# https://towardsdatascience.com/7-tips-for-clean-cmake-scripts-c8d276587389
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
message(FATAL_ERROR "In-source build detected!")
endif()

include(${PROJECT_SOURCE_DIR}/cmake/compiler.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/version.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/dependencies.cmake)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/ccvers.h.in
  ${PROJECT_BINARY_DIR}/version/ccvers.h
  )
include_directories(${CMAKE_CURRENT_BINARY_DIR}/version)

# Core library
add_subdirectory(core)

# calchart_cmd, Mac, Linux only (depends on core)
if(NOT MSVC)
  add_subdirectory(tools/calchart_cmd)
endif()

# CalChart (GUI)
add_subdirectory(src)

# CTests section
if(NOT MSVC)
  add_subdirectory(tests)
  include(CTest)
  add_test(NAME CalChartVersion COMMAND $<TARGET_FILE:calchart_cmd> --version)
  add_test(NAME CalChartTests COMMAND $<TARGET_FILE:CalChartTests>)
  add_test(NAME SanityTest COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/sanity_tester.py -d ${CMAKE_CURRENT_SOURCE_DIR}/shows -g ${CMAKE_CURRENT_SOURCE_DIR}/tests/gold.zip -c $<TARGET_FILE:calchart_cmd>)
endif()

# CPack section
set(CPACK_PACKAGE_NAME CalChart)
set(CPACK_PACKAGE_VENDOR "The University of California Marching Band Computer Comm")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "CalChart, the tool to chart shows for the Cal Band.")
set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
## Ensure all packaging components are available to CPack on all platforms
## so that docs and resources are included in created packages (e.g. macOS DragNDrop DMG).
set(CPACK_COMPONENTS_ALL applications Docs Resources)

# information about the DragNDrop installer was from:
# https://blog.kitware.com/creating-mac-os-x-packages-with-cmake/
include(InstallRequiredSystemLibraries)
set(CPACK_DMG_DS_STORE_SETUP_SCRIPT "${PROJECT_SOURCE_DIR}/resources/macos/CalChart_CMakeDMGSetup.scpt")
set(CPACK_DMG_BACKGROUND_IMAGE "${PROJECT_SOURCE_DIR}/resources/macos/CalChart_DMGBackground.png")

# information about windows NSIS installer
set(CPACK_NSIS_MUI_ICON "${PROJECT_SOURCE_DIR}/resources/common/calchart.ico")
set(CPACK_NSIS_MUI_UNIICON "${PROJECT_SOURCE_DIR}/resources/common/calchart.ico")

set(CPACK_PACKAGE_VERSION_MAJOR "${CalChart_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${CalChart_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${CalChart_VERSION_PATCH}")
set(CPACK_PACKAGE_VERSION "${CalChart_VERSION}")

set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")

include(CPack)

