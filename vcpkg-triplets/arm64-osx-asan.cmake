include("${CMAKE_CURRENT_LIST_DIR}/arm64-osx-calchart.cmake")
set(VCPKG_HASH_ADDITIONAL_FILES "${CMAKE_CURRENT_LIST_DIR}/arm64-osx-calchart.cmake")

# Keep sanitizer instrumentation aligned between the app and vcpkg dependencies.
set(VCPKG_C_FLAGS_DEBUG "${VCPKG_C_FLAGS_DEBUG} -fsanitize=address -fno-omit-frame-pointer")
set(VCPKG_CXX_FLAGS_DEBUG "${VCPKG_CXX_FLAGS_DEBUG} -fsanitize=address -fno-omit-frame-pointer")
set(VCPKG_LINKER_FLAGS_DEBUG "${VCPKG_LINKER_FLAGS_DEBUG} -fsanitize=address")
