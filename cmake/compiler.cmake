# compiler.cmake
# any compiler specific details do them here

if(CMAKE_GENERATOR STREQUAL Xcode)
  set(CMAKE_OSX_ARCHITECTURES "\$(ARCHS_STANDARD)")
endif()

# fix for #348 
if(APPLE)
  set(CMAKE_THREAD_LIBS_INIT "-lpthread")
  set(CMAKE_HAVE_THREADS_LIBRARY 1)
  set(CMAKE_USE_WIN32_THREADS_INIT 0)
  set(CMAKE_USE_PTHREADS_INIT 1)
  set(THREADS_PREFER_PTHREAD_FLAG ON)
endif()

macro(SetupCompilerForTarget arg)
  if(NOT MSVC)
    target_compile_options(${arg} PRIVATE -Wall -Wextra)
  else()
    target_compile_options(${arg} PRIVATE /MP)
  endif()
  set_target_properties(${arg} PROPERTIES CXX_STANDARD 23)
endmacro()

option(ENABLE_ASAN "Enable AddressSanitizer in Debug" ON)
if (CMAKE_BUILD_TYPE STREQUAL "Debug" AND ENABLE_ASAN)
    set(SANITIZER_FLAGS "-fsanitize=address")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SANITIZER_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SANITIZER_FLAGS}")
    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} ${SANITIZER_FLAGS}")
endif()
