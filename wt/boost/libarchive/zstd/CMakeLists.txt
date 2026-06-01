cmake_minimum_required(VERSION 3.10)
set(ZSTD_BUILD_PROGRAMS OFF CACHE BOOL "Force my bool variable to OFF" FORCE)
set(ZSTD_INSTALL_SYMLINKS OFF CACHE BOOL "Force my bool variable to OFF" FORCE)
set(ZSTD_BUILD_STATIC ON CACHE BOOL "Force my bool variable to OFF" FORCE)
set(ZSTD_BUILD_SHARED OFF CACHE BOOL "Force my bool variable to OFF" FORCE)

# Thin wrapper so `cmake -S .` behaves like `cmake -S build/cmake`.
# Policy lives in build/cmake; keep parent project language-less.
project(zstd-superbuild LANGUAGES NONE)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not supported. Specify -B <build-dir>.")
endif()

add_subdirectory(build/cmake)
