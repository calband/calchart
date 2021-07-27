# Testing Library


add_executable(CalChartTests
  ${CMAKE_CURRENT_SOURCE_DIR}/tests/CalChartContinuityTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/tests/CalChartCoordTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/tests/CalChartPointTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/tests/CalChartSheetTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/tests/CalChartShowTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/tests/CalChartTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/tests/CalChartTextTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/tests/PrintToPSTests.cpp

)

set_target_properties (CalChartTests PROPERTIES CXX_STANDARD 20)
target_include_directories (
  CalChartTests
  PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/submodules/Catch2/single_include"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/core"
)
target_link_libraries (
  CalChartTests
  calchart_core
  nlohmann_json::nlohmann_json
)



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

target_sources(CalChart PRIVATE resources-windows/WindowsHiDPI.manifest)

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
set (CPACK_DMG_DS_STORE_SETUP_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/resources-macos/CalChart_CMakeDMGSetup.scpt")
set (CPACK_DMG_BACKGROUND_IMAGE "${CMAKE_CURRENT_SOURCE_DIR}/resources-macos/CalChart_DMGBackground.png")

set (CPACK_PACKAGE_VERSION_MAJOR "${CalChart_VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${CalChart_VERSION_MINOR}")
set (CPACK_PACKAGE_VERSION_PATCH "${CalChart_VERSION_PATCH}")
set (CPACK_PACKAGE_VERSION "${CalChart_VERSION}")

set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")

include (CPack)

# tests for calchart_core
include(CTest)
add_test (TutorialRuns calchart_cmd --version)
add_test (CalChartTests CalChartTests)


