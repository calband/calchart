# version.cmake

execute_process(COMMAND git describe --tags
                OUTPUT_VARIABLE GIT_VERS
                )
message ("Found version  " ${GIT_VERS})
# get rid of 'v'
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


