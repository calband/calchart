# CalChart CMake

# CalChart docs (excludes developer-only docs in DeveloperDocs/)
file(
  GLOB CalChartDocs
    "${PROJECT_SOURCE_DIR}/docs/*"
)

# Split CalChartDocs into plain files and directories so we can install them
# appropriately (install FILES cannot be given directories).
set(CalChartDocFiles)
set(CalChartDocDirs)
foreach(_doc ${CalChartDocs})
  if(IS_DIRECTORY ${_doc})
    list(APPEND CalChartDocDirs ${_doc})
  else()
    list(APPEND CalChartDocFiles ${_doc})
  endif()
endforeach()

file(
  GLOB CalChartResources
    "${PROJECT_SOURCE_DIR}/resources/common/*"
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
  AnimationSprites.cpp
  AnimationSprites.hpp
  BackgroundImages.cpp
  BackgroundImages.h
  BugReportDialog.cpp
  BugReportDialog.h
  GitHubIssueSubmitter.cpp
  GitHubIssueSubmitter.hpp
  CCOmniviewCanvas.cpp
  CCOmniviewCanvas.h
  CalChartApp.cpp
  CalChartApp.h
  CalChartCoordHelper.h
  CalChartLogTarget.cpp
  CalChartLogTarget.h
  CalChartDoc.cpp
  CalChartDoc.h
  CalChartDocCommand.cpp
  CalChartDocCommand.h
  CalChartDrawing.h
  DiagnosticInfo.cpp
  DiagnosticInfo.h
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
  HelpManager.cpp
  HelpManager.hpp
  HelpDialog.cpp
  HelpDialog.hpp
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
  WebViewDemoDialog.cpp
  WebViewDemoDialog.h
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
  "${PROJECT_SOURCE_DIR}/resources/common"
)

target_link_libraries(
  CalChart
  calchart_core
  nlohmann_json::nlohmann_json
  ${wxWidgets_LIBRARIES}
  wxUI
  WebP::webp
  WebP::webpdemux
)

# Add WebView2 support for static Windows builds
# The WebView2 package may be installed via nuget and its location specified
# through WEBVIEW2_LIB_PATH cmake cache variable or environment variable
if(WIN32 AND NOT BUILD_SHARED_LIBS)
  if(DEFINED ENV{WEBVIEW2_LIB_PATH} AND NOT WEBVIEW2_LIB_PATH)
    set(WEBVIEW2_LIB_PATH "$ENV{WEBVIEW2_LIB_PATH}" CACHE PATH "Path to WebView2 loader static library directory" FORCE)
  endif()

  if(WEBVIEW2_LIB_PATH)
    find_library(WEBVIEW2_LOADER_LIB
      NAMES WebView2LoaderStatic
      PATHS "${WEBVIEW2_LIB_PATH}"
      NO_DEFAULT_PATH
    )
    if(WEBVIEW2_LOADER_LIB)
      target_link_libraries(CalChart ${WEBVIEW2_LOADER_LIB})
      message(STATUS "Found WebView2LoaderStatic: ${WEBVIEW2_LOADER_LIB}")
    else()
      message(WARNING "WEBVIEW2_LIB_PATH set but WebView2LoaderStatic.lib not found in: ${WEBVIEW2_LIB_PATH}")
    endif()
  else()
    message(STATUS "WEBVIEW2_LIB_PATH not set; WebView2 may not link correctly on Windows static builds")
  endif()
endif()

# Special stuff for Mac
if(APPLE)

set_target_properties(
  CalChart PROPERTIES
  MACOSX_BUNDLE_INFO_PLIST "${PROJECT_SOURCE_DIR}/resources/macos/CalChart-Info.plist.in"
  MACOSX_BUNDLE_ICON_FILE calchart.icns
  MACOSX_BUNDLE TRUE
  MACOSX_FRAMEWORK_IDENTIFIER com.calband.CalChart
  MACOSX_BUNDLE_GUI_IDENTIFIER com.calband.CalChart
  MACOSX_BUNDLE_BUNDLE_NAME CalChart
  MACOSX_BUNDLE_BUNDLE_VERSION ${CalChart_VERSION}
  MACOSX_BUNDLE_SHORT_VERSION_STRING ${CalChart_VERSION}
  MACOSX_BUNDLE_LONG_VERSION_STRING ${CalChart_VERSION}
)

# Ensure CalChart rebuilds when help HTML is updated
if(TARGET build_help_html)
  add_dependencies(CalChart build_help_html)
  
  # Make CalChart depend on the stamp file so it knows when HTML was regenerated
  set(HELP_STAMP_FILE "${CMAKE_BINARY_DIR}/docs/.html_built")
  if(EXISTS "${HELP_STAMP_FILE}" OR TARGET build_help_html)
    add_custom_command(
      OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/help_html_copied.stamp"
      COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_BINARY_DIR}/docs/html" "$<TARGET_FILE_DIR:CalChart>/../Resources/docs/html"
      COMMAND ${CMAKE_COMMAND} -E touch "${CMAKE_CURRENT_BINARY_DIR}/help_html_copied.stamp"
      DEPENDS ${HELP_STAMP_FILE}
      COMMENT "Copying pre-built HTML help files to app bundle"
    )
    add_custom_target(copy_help_html ALL
      DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/help_html_copied.stamp"
    )
    add_dependencies(copy_help_html build_help_html CalChart)
  endif()
endif()

# Copy docs directory to the bundle's Resources
add_custom_command(
  TARGET CalChart POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${PROJECT_SOURCE_DIR}/docs" "$<TARGET_FILE_DIR:CalChart>/../Resources/docs"
)


endif()

# special stuff for Windows
if(WIN32)

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT "CalChart")
set_target_properties(CalChart PROPERTIES WIN32_EXECUTABLE TRUE)
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

# Copy generated HTML docs
add_custom_command(
  TARGET CalChart POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${CMAKE_BINARY_DIR}/docs/html" $<TARGET_FILE_DIR:CalChart>/docs/html
  DEPENDS build_help_html
)

target_sources(CalChart PRIVATE ${PROJECT_SOURCE_DIR}/resources/windows/WindowsHiDPI.manifest)

endif ()

# Copy docs for Linux/Unix (non-Windows, non-Apple)
if(NOT WIN32 AND NOT APPLE)
add_custom_command(
  TARGET CalChart POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${PROJECT_SOURCE_DIR}/docs" "$<TARGET_FILE_DIR:CalChart>/docs"
)

# Copy generated HTML docs
add_custom_command(
  TARGET CalChart POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${CMAKE_BINARY_DIR}/docs/html" "$<TARGET_FILE_DIR:CalChart>/docs/html"
  DEPENDS build_help_html
)
endif()

# we add some targets for docs and images so we can pack them for windows
# see http://yanivresearch.info/software/CPackWindowsTutorial/CPackForWindows.html
## Install top-level doc files and the docs/source directory (if present)
if(CalChartDocFiles)
  install(
    FILES
      ${CalChartDocFiles}
    DESTINATION docs
    COMPONENT Docs
  )
endif()

# Install any subdirectories found under docs/ (e.g. docs/source)
foreach(_docdir ${CalChartDocDirs})
  get_filename_component(_basename ${_docdir} NAME)
  install(
    DIRECTORY "${_docdir}/"
    DESTINATION docs/${_basename}
    COMPONENT Docs
  )
endforeach()

install(
  FILES ${CalChartResources}
  DESTINATION resources
  COMPONENT Resources
)

if(NOT MSVC)
  add_subdirectory(tests)
endif()
