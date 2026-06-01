# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.30.5)

# Reject any attempt to use a toolchain file. We must not use one because
# we could be downloading it here. If the CMAKE_TOOLCHAIN_FILE environment
# variable is set, the cache variable will have been initialized from it.
unset(CMAKE_TOOLCHAIN_FILE CACHE)
unset(ENV{CMAKE_TOOLCHAIN_FILE})

# We name the project and the target for the ExternalProject_Add() call
# to something that will highlight to the user what we are working on if
# something goes wrong and an error message is produced.

project(fetchgooglefontcmake-populate NONE)


# Pass through things we've already detected in the main project to avoid
# paying the cost of redetecting them again in ExternalProject_Add()
set(GIT_EXECUTABLE [==[C:/Git/cmd/git.exe]==])
set(GIT_VERSION_STRING [==[2.51.0.windows.2]==])
set_property(GLOBAL PROPERTY _CMAKE_FindGit_GIT_EXECUTABLE_VERSION
  [==[C:/Git/cmd/git.exe;2.51.0.windows.2]==]
)


include(ExternalProject)
ExternalProject_Add(fetchgooglefontcmake-populate
                     "UPDATE_DISCONNECTED" "False" "GIT_REPOSITORY" "https://github.com/OlivierLDff/FetchGoogleFontCMake.git" "EXTERNALPROJECT_INTERNAL_ARGUMENT_SEPARATOR" "GIT_TAG" "c19d83a68eda83b9dd3033893dbb63d84c941196"
                    SOURCE_DIR          "C:/Qt/dev/qaterialdev/build/Desktop_Qt_6_9_3_MSVC2022_64bit-Release/_deps/fetchgooglefontcmake-src"
                    BINARY_DIR          "C:/Qt/dev/qaterialdev/build/Desktop_Qt_6_9_3_MSVC2022_64bit-Release/_deps/fetchgooglefontcmake-build"
                    CONFIGURE_COMMAND   ""
                    BUILD_COMMAND       ""
                    INSTALL_COMMAND     ""
                    TEST_COMMAND        ""
                    USES_TERMINAL_DOWNLOAD  YES
                    USES_TERMINAL_UPDATE    YES
                    USES_TERMINAL_PATCH     YES
)


