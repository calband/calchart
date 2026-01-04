# CalChart Documentation Build

# Convert Markdown to HTML for help system
# Requires pandoc to be installed (see GETTING_STARTED.md for platform-specific instructions)
set(HELP_MD_DIR "${PROJECT_SOURCE_DIR}/docs/md")
set(HELP_HTML_DIR "${CMAKE_BINARY_DIR}/docs/html")
set(HELP_STAMP_FILE "${CMAKE_BINARY_DIR}/docs/.html_built")

# Find all markdown files to establish dependencies
file(GLOB_RECURSE MARKDOWN_FILES "${HELP_MD_DIR}/*.md")

# Create a custom target to convert markdown to HTML using our conversion script
if(Python3_FOUND OR Python_FOUND)
  set(PYTHON_EXECUTABLE "${Python3_EXECUTABLE}" CACHE FILEPATH "Python executable")
  if(NOT PYTHON_EXECUTABLE)
    set(PYTHON_EXECUTABLE "${Python_EXECUTABLE}")
  endif()
  
  add_custom_command(
    OUTPUT ${HELP_STAMP_FILE}
    COMMAND ${PYTHON_EXECUTABLE} "${PROJECT_SOURCE_DIR}/scripts/convert_markdown_to_html.py" "${HELP_MD_DIR}" "${HELP_HTML_DIR}"
    COMMAND ${CMAKE_COMMAND} -E touch ${HELP_STAMP_FILE}
    DEPENDS ${MARKDOWN_FILES}
    COMMENT "Converting Markdown help files to HTML using pandoc"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
  )
  
  add_custom_target(build_help_html ALL
    DEPENDS ${HELP_STAMP_FILE}
  )
else()
  message(WARNING "Python not found - help HTML conversion will be skipped. Install Python to build help documentation.")
endif()
