# ARM64 Windows overlay triplet for CalChart.
# This file name must match VCPKG_TARGET_TRIPLET=arm64-windows.
# Overlay triplets fully replace built-in triplets, so we must define the
# standard triplet identity/linkage values here.
set(VCPKG_TARGET_ARCHITECTURE arm64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE dynamic)

include("${CMAKE_CURRENT_LIST_DIR}/windows.cmake")
