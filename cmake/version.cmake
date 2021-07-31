# version.cmake
# This 

# This will produce a string of the form v3.6.2-48-g6538532
# which is the tag plus number of changes since the tag if any
execute_process(COMMAND git describe --tags
                OUTPUT_VARIABLE GIT_VERS
                )

# decompose the tag string into: 'v' Major '.' Minor '.' Patch
string(STRIP ${GIT_VERS} GIT_VERS)
string(SUBSTRING ${GIT_VERS} 1 -1 GIT_VERS)
string(FIND ${GIT_VERS} "." bound)
string(SUBSTRING ${GIT_VERS} 0 ${bound} Found_MAJOR)
string(SUBSTRING ${GIT_VERS} ${bound} -1 GIT_VERS)
string(SUBSTRING ${GIT_VERS} 1 -1 GIT_VERS)
string(FIND ${GIT_VERS} "." bound)
string(SUBSTRING ${GIT_VERS} 0 ${bound} Found_MINOR)
string(SUBSTRING ${GIT_VERS} ${bound} -1 GIT_VERS)
string(SUBSTRING ${GIT_VERS} 1 -1 GIT_VERS)
set (Found_PATCH ${GIT_VERS})

set (CalChart_VERSION_MAJOR ${Found_MAJOR})
set (CalChart_VERSION_MINOR ${Found_MINOR})
set (CalChart_VERSION_PATCH ${Found_PATCH})
set (CalChart_VERSION "${CalChart_VERSION_MAJOR}.${CalChart_VERSION_MINOR}.${CalChart_VERSION_PATCH}")

