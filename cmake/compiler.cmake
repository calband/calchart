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

option(CORE_WARNINGS_AS_ERRORS "Treat warnings as errors in calchart_core" ON)
option(ENABLE_ASAN "Enable AddressSanitizer in Debug" ON)
if (CMAKE_BUILD_TYPE STREQUAL "Debug" AND ENABLE_ASAN)
    set(SANITIZER_FLAGS "-fsanitize=address")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SANITIZER_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SANITIZER_FLAGS}")
    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} ${SANITIZER_FLAGS}")
endif()

# On Windows (MSVC), ensure FetchContent dependencies use the same runtime library
# as the parent project to avoid linker errors (CMAKE_MSVC_RUNTIME_LIBRARY).
# This requires CMake policy CMP0091 to be NEW (introduced in CMake 3.15).
if(MSVC)
  # Set policy default for dependencies that don't specify it
  set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)
  # Use dynamic runtime library (/MD for Release, /MDd for Debug)
  # This matches typical Windows builds and vcpkg defaults
  if(NOT DEFINED CMAKE_MSVC_RUNTIME_LIBRARY)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
  endif()
endif()
