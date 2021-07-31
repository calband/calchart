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
  endif()
  set_target_properties(${arg} PROPERTIES CXX_STANDARD 20)
endmacro()

