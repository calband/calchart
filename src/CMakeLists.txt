# CalChart CMake

# CalChart
add_subdirectory(core)

file(
  GLOB CalChartDocs
    "${PROJECT_SOURCE_DIR}/docs/*"
)

file(
  GLOB CalChartResources
    "${PROJECT_SOURCE_DIR}/resources/*"
)

source_group(
  "Resources" FILES
  ${CalChartDocs}
  ${CalChartResources}
)

add_executable(
  CalChart
  AnimationCanvas.cpp
  AnimationCanvas.h
  AnimationErrorsPanel.cpp
  AnimationErrorsPanel.h
  AnimationPanel.cpp
  AnimationPanel.h
  AnimationView.cpp
  AnimationView.h
  BackgroundImages.cpp
  BackgroundImages.h
  CCOmniviewCanvas.cpp
  CCOmniviewCanvas.h
  CalChartApp.cpp
  CalChartApp.h
  CalChartCoordHelper.h
  CalChartDoc.cpp
  CalChartDoc.h
  CalChartDocCommand.cpp
  CalChartDocCommand.h
  CalChartDrawing.h
  CalChartDrawing.cpp
  CalChartDrawingGetMinSize.h
  CalChartDrawingLayout.h
  CalChartDrawPrimativesHelper.h
  CalChartFrame.cpp
  CalChartFrame.h
  CalChartPreferences.cpp
  CalChartPreferences.h
  CalChartSizes.cpp
  CalChartSizes.h
  CalChartSplash.cpp
  CalChartSplash.h
  CalChartToolBar.cpp
  CalChartToolBar.h
  CalChartView.cpp
  CalChartView.h
  ColorPalette.cpp
  ColorPalette.h
  ColorSetupCanvas.cpp
  ColorSetupCanvas.h
  ColorSetupDialog.cpp
  ColorSetupDialog.h
  ConfigurationDebugDialog.cpp
  ConfigurationDebugDialog.h
  ContinuityBoxDrawer.cpp
  ContinuityBoxDrawer.h
  ContinuityBrowser.cpp
  ContinuityBrowser.h
  ContinuityBrowserPanel.cpp
  ContinuityBrowserPanel.h
  ContinuityComposerDialog.cpp
  ContinuityComposerDialog.h
  ContinuityEditorPopup.cpp
  ContinuityEditorPopup.h
  CustomListViewPanel.cpp
  CustomListViewPanel.h
  EditCurveAssignments.cpp
  EditCurveAssignments.hpp
  DCSaveRestore.h
  FieldCanvas.cpp
  FieldCanvas.h
  FieldControlsToolBar.cpp
  FieldControlsToolBar.h
  FieldThumbnailBrowser.cpp
  FieldThumbnailBrowser.h
  HostAppInterface.cpp
  HostAppInterface.h
  ModeSetupDialog.cpp
  ModeSetupDialog.h
  PointPicker.cpp
  PointPicker.h
  PreferencesContCellSetup.cpp
  PreferencesContCellSetup.h
  PreferencesDrawingSetup.cpp
  PreferencesDrawingSetup.h
  PreferencesGeneralSetup.cpp
  PreferencesGeneralSetup.h
  PreferencesPrintContinuitySetup.cpp
  PreferencesPrintContinuitySetup.h
  PreferencesPSPrintingSetup.cpp
  PreferencesPSPrintingSetup.h
  PreferencesShowModeSetup.cpp
  PreferencesShowModeSetup.h
  PreferencesUtils.cpp
  PreferencesUtils.h
  PrintContinuityEditor.cpp
  PrintContinuityEditor.h
  PrintContinuityPreview.cpp
  PrintContinuityPreview.h
  PrintPostScriptDialog.cpp
  PrintPostScriptDialog.h
  SetupInstruments.cpp
  SetupInstruments.h
  SetupMarchers.cpp
  SetupMarchers.h
  ShowModeSetupCanvas.cpp
  ShowModeSetupCanvas.h
  ShowModeWizard.cpp
  ShowModeWizard.h
  StackDrawPlayground.cpp
  StackDrawPlayground.h
  SystemConfiguration.cpp
  SystemConfiguration.h
  TransitionSolverFrame.cpp
  TransitionSolverFrame.h
  TransitionSolverProgressFrame.cpp
  TransitionSolverProgressFrame.h
  TransitionSolverView.cpp
  TransitionSolverView.h
  basic_ui.cpp
  basic_ui.h
  calchart.rc
  platconf.h
  ui_enums.h
  ${CalChartDocs}
  ${CalChartResources}
)

# it is important to put this before we set the resources as the CPack for Mac likes to have a minimum set of files to look good.
install(
  TARGETS CalChart
  DESTINATION .
  COMPONENT applications
)

set(AllResources ${CalChartDocs} ${CalChartResources}) 

set_target_properties(
  CalChart PROPERTIES
  RESOURCE
  "${AllResources}"
)

SetupCompilerForTarget(CalChart)

target_include_directories(
  CalChart
  PRIVATE
  "${PROJECT_SOURCE_DIR}/resources"
)

target_link_libraries(
  CalChart
  calchart_core
  nlohmann_json::nlohmann_json
  wx::gl wx::core wx::base wx::adv wx::html wx::net wx::aui
  wxUI
)

# Special stuff for Mac
if(APPLE)

set_target_properties(
  CalChart PROPERTIES
  MACOSX_BUNDLE_INFO_PLIST "${PROJECT_SOURCE_DIR}/resources-macos/CalChart-Info.plist.in"
  MACOSX_BUNDLE_ICON_FILE calchart.icns
  MACOSX_BUNDLE TRUE
  MACOSX_FRAMEWORK_IDENTIFIER com.calband.CalChart
  MACOSX_BUNDLE_GUI_IDENTIFIER com.calband.CalChart
  MACOSX_BUNDLE_BUNDLE_NAME CalChart
  MACOSX_BUNDLE_BUNDLE_VERSION ${CalChart_VERSION}
  MACOSX_BUNDLE_SHORT_VERSION_STRING ${CalChart_VERSION}
  MACOSX_BUNDLE_LONG_VERSION_STRING ${CalChart_VERSION}
)

endif()

# special stuff for Windows
if(MSVC)

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT "CalChart")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
add_custom_command(
  TARGET CalChart POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${PROJECT_SOURCE_DIR}/resources" $<TARGET_FILE_DIR:CalChart>/resources
)

add_custom_command(
  TARGET CalChart POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${PROJECT_SOURCE_DIR}/docs" $<TARGET_FILE_DIR:CalChart>/docs
)

target_sources(CalChart PRIVATE ${PROJECT_SOURCE_DIR}/resources-windows/WindowsHiDPI.manifest)

endif ()

# we add some targets for docs and images so we can pack them for windows
# see http://yanivresearch.info/software/CPackWindowsTutorial/CPackForWindows.html
install(
  FILES ${CalChartDocs}
  DESTINATION docs
  COMPONENT Docs
)

install(
  FILES ${CalChartResources}
  DESTINATION resources
  COMPONENT Resources
)


