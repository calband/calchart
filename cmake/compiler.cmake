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
  set_target_properties(${arg} PROPERTIES CXX_STANDARD 23)
endmacro()

##
# MSVC runtime selection
#
# When building against vcpkg's x64-windows-static triplet, dependencies like
# wxWidgets are built with the static CRT (/MT*). Ensure our targets also use
# the static runtime to avoid LNK2038 mismatches (MD vs MT).
#
# Prefer the modern CMake knob when available (3.15+), otherwise fall back to
# replacing flags for older CMake versions (our minimum is 3.11).
if(MSVC)
  # Enable policy so CMAKE_MSVC_RUNTIME_LIBRARY is honored when supported
  if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
  endif()

  if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.15")
    # Use static runtime for all configs; adds /MTd for Debug, /MT for others
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  else()
    # Fallback: rewrite /MD to /MT across C and C++ flags for all configs
    foreach(flag_var
            CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
            CMAKE_C_FLAGS_RELWITHDEBINFO CMAKE_C_FLAGS_MINSIZEREL
            CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
            CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_CXX_FLAGS_MINSIZEREL)
      if(DEFINED ${flag_var})
        string(REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
      endif()
    endforeach()
  endif()
endif()

option(ENABLE_ASAN "Enable AddressSanitizer in Debug" ON)
if (CMAKE_BUILD_TYPE STREQUAL "Debug" AND ENABLE_ASAN)
    set(SANITIZER_FLAGS "-fsanitize=address")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SANITIZER_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SANITIZER_FLAGS}")
    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} ${SANITIZER_FLAGS}")
endif()
