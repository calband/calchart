# Common Windows overlay-triplet flags for CalChart.
# Keep this file as a shared include and use arch-specific triplet files
# (for example arm64-windows.cmake) to include it.
set(VCPKG_CXX_FLAGS "/Zc:__cplusplus /std:c++20")
set(VCPKG_C_FLAGS "/Zc:__cplusplus")

