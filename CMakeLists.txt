# CMakeLists.txt

# some great sources to read to understand CMake:
# CMake Tutorial (https://cmake.org/cmake-tutorial/)
# CGold (https://cgold.readthedocs.io/en/latest/index.html)
# Effective Modern CMake (https://gist.github.com/mbinna/c61dbb39bca0e4fb7d1f73b0d66a4fd1)
# Creating Mac OS X Packages with CMake (https://blog.kitware.com/creating-mac-os-x-packages-with-cmake/)

cmake_minimum_required (VERSION 3.11)

project (CalChart)

set (CalChart_VERSION_MAJOR 3)
set (CalChart_VERSION_MINOR 6)
set (CalChart_VERSION_PATCH 2)

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

set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory (submodules/nlohmann_json EXCLUDE_FROM_ALL)

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
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/CalChartAnimation.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/CalChartAnimation.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/CalChartAnimationErrors.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/CalChartAnimationSheet.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/CalChartAnimationSheet.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/CalChartAnimationCommand.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/CalChartAnimationCommand.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/CalChartAnimationCompile.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/CalChartAnimationCompile.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/CalChartCoord.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/CalChartCoord.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/CalChartMovePointsTool.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/CalChartMovePointsTool.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/CalChartSelectTool.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/CalChartSelectTool.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/CalChartShapes.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/CalChartShapes.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/CalChartTypes.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/animate_types.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/cc_continuity.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/cc_continuity.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/cc_drawcommand.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/cc_fileformat.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/cc_image.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/cc_image.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/cc_parse_errors.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/cc_point.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/cc_point.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/cc_sheet.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/cc_sheet.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/cc_show.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/cc_show.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/cc_text.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/cc_text.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/cont.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/cont.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/e7_transition_solver.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/e7_transition_solver.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/linmath.h
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
if (MSVC)
target_compile_options (calchart_core PRIVATE -Wall)
else ()
target_compile_options (calchart_core PRIVATE -Werror -Wall -Wextra)
endif ()

set_target_properties (calchart_core PROPERTIES CXX_STANDARD 20)

target_include_directories (
  calchart_core
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core
  ${Boost_INCLUDE_DIRS}
  ${PROJECT_BINARY_DIR}/version
  ${CMAKE_CURRENT_SOURCE_DIR}/submodules/munkres-cpp/src
  )

target_link_libraries(calchart_core PRIVATE nlohmann_json::nlohmann_json)

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

set_target_properties (calchart_cmd PROPERTIES CXX_STANDARD 17)

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
  GLOB CalChartDocs
    "${CMAKE_CURRENT_SOURCE_DIR}/docs/*"
  )

file (
  GLOB CalChartResources
    "${CMAKE_CURRENT_SOURCE_DIR}/resources/*"
  )

source_group(
  "Resources" FILES
  ${CalChartDocs}
  ${CalChartResources}
  )

add_executable (
  CalChart
  ${CMAKE_CURRENT_SOURCE_DIR}/src/AnimationCanvas.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/AnimationCanvas.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/AnimationErrorsPanel.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/AnimationErrorsPanel.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/AnimationPanel.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/AnimationPanel.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/AnimationView.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/AnimationView.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/BackgroundImage.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/BackgroundImage.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/CCOmniviewCanvas.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/CCOmniviewCanvas.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/CalChartApp.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/CalChartApp.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/CalChartDoc.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/CalChartDoc.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/CalChartDocCommand.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/CalChartDocCommand.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/CalChartFrame.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/CalChartFrame.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/CalChartPreferences.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/CalChartPreferences.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/CalChartSizes.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/CalChartSizes.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/CalChartSplash.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/CalChartSplash.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/CalChartToolBar.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/CalChartToolBar.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/CalChartView.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/CalChartView.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ColorPalette.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ColorPalette.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ColorSetupCanvas.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ColorSetupCanvas.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ColorSetupDialog.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ColorSetupDialog.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ContinuityBrowser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ContinuityBrowser.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ContinuityBrowserPanel.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ContinuityBrowserPanel.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ContinuityComposerDialog.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ContinuityComposerDialog.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ContinuityEditorPopup.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ContinuityEditorPopup.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/CustomListViewPanel.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/CustomListViewPanel.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/FieldCanvas.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/FieldCanvas.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/FieldControlsToolBar.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/FieldControlsToolBar.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/FieldThumbnailBrowser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/FieldThumbnailBrowser.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/HostAppInterface.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/HostAppInterface.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/PointPicker.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/PointPicker.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/PreferencesContCellSetup.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/PreferencesContCellSetup.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/PreferencesDrawingSetup.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/PreferencesDrawingSetup.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/PreferencesGeneralSetup.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/PreferencesGeneralSetup.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/PreferencesPSPrintingSetup.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/PreferencesPSPrintingSetup.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/PreferencesShowModeSetup.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/PreferencesShowModeSetup.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/PreferencesUtils.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/PreferencesUtils.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/PrintContinuityEditor.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/PrintContinuityEditor.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/PrintPostScriptDialog.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/PrintPostScriptDialog.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/SetupInstruments.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/SetupInstruments.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/SetupMarchers.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/SetupMarchers.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/TransitionSolverFrame.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/TransitionSolverFrame.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/TransitionSolverProgressFrame.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/TransitionSolverProgressFrame.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/TransitionSolverView.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/TransitionSolverView.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/basic_ui.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/basic_ui.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc_omniview_constants.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/confgr.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/confgr.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cont_box_draw.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cont_box_draw.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/draw.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/draw.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/draw_utils.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/GhostModule.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/mode_dialog.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/mode_dialog.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/mode_dialog_canvas.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/mode_dialog_canvas.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/platconf.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/setup_wizards.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/setup_wizards.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ui_enums.h
  ${CalChartDocs}
  ${CalChartResources}
  "${CMAKE_CURRENT_SOURCE_DIR}/calchart.rc"
  )

install (
  TARGETS CalChart
  DESTINATION .
  COMPONENT applications
  )


set(AllResources ${CalChartDocs} ${CalChartResources}) 

set_target_properties (
  CalChart PROPERTIES
  RESOURCE
  "${AllResources}"
  )

target_link_libraries (CalChart calchart_core ${Boost_LIBRARIES})
target_link_libraries (CalChart nlohmann_json::nlohmann_json)

#target_link_libraries (
#  CalChart
#  PRIVATE
#  calchart_core
#  ${Boost_LIBRARIES}
#  nlohmann_json::nlohmann_json
#  )


# We include wxWidgets as a submodule (see https://docs.wxwidgets.org/trunk/overview_cmake.html for info)
# Have wxWidgets build as static libraries
set (wxBUILD_SHARED OFF)
set (wxUSE_STL ON)
set (wxUSE_STD_CONTAINERS ON)
add_subdirectory (submodules/wxWidgets)

target_link_libraries (
  CalChart wx::gl wx::core wx::base wx::adv wx::html wx::net wx::aui
  )

set_target_properties (CalChart PROPERTIES CXX_STANDARD 17)

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
  "${CMAKE_CURRENT_SOURCE_DIR}/resources" $<TARGET_FILE_DIR:CalChart>/resources
  )

add_custom_command (
  TARGET CalChart POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${CMAKE_CURRENT_SOURCE_DIR}/docs" $<TARGET_FILE_DIR:CalChart>/docs
  )

target_sources(CalChart PRIVATE build-windows/WindowsHiDPI.manifest)

endif ()

# we add some targets for docs and images so we can pack them for windows
# see http://yanivresearch.info/software/CPackWindowsTutorial/CPackForWindows.html
install (
  FILES ${CalChartDocs}
  DESTINATION docs
  COMPONENT Docs
  )

install (
  FILES ${CalChartResources}
  DESTINATION resources
  COMPONENT Resources
  )

# Installer section
# information about the DragNDrop installer was from:
# https://blog.kitware.com/creating-mac-os-x-packages-with-cmake/

include (InstallRequiredSystemLibraries)

set (CPACK_PACKAGE_NAME CalChart)
set (CPACK_ARCHIVE_COMPONENT_INSTALL ON)
if (MSVC)
  set (CPACK_COMPONENTS_ALL applications Docs Resources)
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


