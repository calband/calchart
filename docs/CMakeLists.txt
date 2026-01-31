# CalChart Documentation Build

# Convert Markdown to HTML for help system
# Requires pandoc to be installed (see GETTING_STARTED.md for platform-specific instructions)
set(HELP_MD_DIR "${PROJECT_SOURCE_DIR}/docs/md")
set(HELP_HTML_DIR "${CMAKE_BINARY_DIR}/docs/html")
set(HELP_STAMP_FILE "${CMAKE_BINARY_DIR}/docs/.html_built")

# Find all markdown files to establish dependencies
file(GLOB_RECURSE MARKDOWN_FILES "${HELP_MD_DIR}/*.md")

# Require Python for help system build
if(NOT Python3_FOUND AND NOT Python_FOUND)
  message(FATAL_ERROR "Python is required to build CalChart help documentation. Please install Python 3 and ensure it is in your PATH. See GETTING_STARTED.md for installation instructions.")
endif()

set(PYTHON_EXECUTABLE "${Python3_EXECUTABLE}" CACHE FILEPATH "Python executable")
if(NOT PYTHON_EXECUTABLE)
  set(PYTHON_EXECUTABLE "${Python_EXECUTABLE}")
endif()

# Require pandoc for help system build
find_program(PANDOC_EXECUTABLE pandoc)
if(NOT PANDOC_EXECUTABLE)
  message(FATAL_ERROR "Pandoc is required to build CalChart help documentation. Please install pandoc and ensure it is in your PATH.\n  - macOS: brew install pandoc\n  - Windows: choco install pandoc\n  - Linux: sudo apt-get install pandoc\nSee GETTING_STARTED.md for more details.")
endif()

# Create a custom target to convert markdown to HTML using our conversion script
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

# Install the generated HTML files
install(
  DIRECTORY ${HELP_HTML_DIR}/
  DESTINATION docs/html
  COMPONENT Docs
)
