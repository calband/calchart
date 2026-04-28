# version.cmake
# This 

# This will produce a string of the form v3.6.2-48-g6538532
# which is the tag plus number of changes since the tag if any
execute_process(COMMAND git describe --tags --always --dirty
                OUTPUT_VARIABLE GIT_VERS
                OUTPUT_STRIP_TRAILING_WHITESPACE
                )

# Save the full git version for diagnostic purposes
set (CalChart_GIT_VERSION "${GIT_VERS}")

# decompose the tag string into: 'v' Major '.' Minor '.' Patch
# Strip the leading 'v' if present
set (GIT_VERS_STRIPPED "${GIT_VERS}")
if (GIT_VERS_STRIPPED MATCHES "^v")
  string(SUBSTRING ${GIT_VERS_STRIPPED} 1 -1 GIT_VERS_STRIPPED)
endif()

# Extract major version
string(FIND ${GIT_VERS_STRIPPED} "." bound)
if (bound GREATER -1)
  string(SUBSTRING ${GIT_VERS_STRIPPED} 0 ${bound} Found_MAJOR)
  string(SUBSTRING ${GIT_VERS_STRIPPED} ${bound} -1 GIT_VERS_STRIPPED)
  string(SUBSTRING ${GIT_VERS_STRIPPED} 1 -1 GIT_VERS_STRIPPED)
else()
  set(Found_MAJOR 0)
endif()

# Extract minor version
string(FIND ${GIT_VERS_STRIPPED} "." bound)
if (bound GREATER -1)
  string(SUBSTRING ${GIT_VERS_STRIPPED} 0 ${bound} Found_MINOR)
  string(SUBSTRING ${GIT_VERS_STRIPPED} ${bound} -1 GIT_VERS_STRIPPED)
  string(SUBSTRING ${GIT_VERS_STRIPPED} 1 -1 GIT_VERS_STRIPPED)
else()
  set(Found_MINOR 0)
endif()

# Rest is patch (may include -commits-ghash)
set (Found_PATCH ${GIT_VERS_STRIPPED})

set (CalChart_VERSION_MAJOR ${Found_MAJOR})
set (CalChart_VERSION_MINOR ${Found_MINOR})
set (CalChart_VERSION_PATCH ${Found_PATCH})
set (CalChart_VERSION "${CalChart_VERSION_MAJOR}.${CalChart_VERSION_MINOR}.${CalChart_VERSION_PATCH}")

