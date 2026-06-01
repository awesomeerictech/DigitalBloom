# SPDX-License-Identifier: MIT
#
# SPDX-FileCopyrightText: Copyright (c) 2019-2023 Lars Melchior and contributors

# Heuristic to check if file have already been found
if(COMMAND CpmAddPackage)
  return()
endif()

#[[if(ANDROID)
    # For Android builds, CMake restricts the search to the root path.
    # We must explicitly add our project's source directory to that search path.
    list(APPEND CMAKE_FIND_ROOT_PATH "${CMAKE_SOURCE_DIR}")
endif()]]

#[[ Add these debug messages *before* the find_file call.
message(STATUS "CMAKE_BINARY_DIR is: ${CMAKE_BINARY_DIR}")
message(STATUS "CMAKE_SOURCE_DIR is: ${CMAKE_SOURCE_DIR}")
message(STATUS "CMAKE_CURRENT_LIST_DIR is: ${CMAKE_CURRENT_LIST_DIR}")
message(STATUS "QATERIAL_USE_LOCAL_CPM_FILE is: ${QATERIAL_USE_LOCAL_CPM_FILE}")
set(FILE_TO_CHECK "${CMAKE_SOURCE_DIR}/cmake/CPM_0.40.2.cmake")

if(EXISTS "${FILE_TO_CHECK}")
  message(STATUS "File exists at: ${FILE_TO_CHECK}")
else()
  message(STATUS "File does NOT exist at: ${FILE_TO_CHECK}")
endif()]]

#[[set(CPM_SEARCH_PATHS
    "${CMAKE_BINARY_DIR}"
    "${CMAKE_SOURCE_DIR}/cmake"
	"${CMAKE_CURRENT_LIST_DIR}"
	"${CMAKE_BINARY_DIR}/eric/qaterial-src/cmake"
)]]

set(CPM_SEARCH_PATHS "CMAKE_CURRENT_LIST_DIR" )
message(STATUS "CMAKE_CURRENT_LIST_DIR is: ${CMAKE_CURRENT_LIST_DIR}")

#[[if(QATERIAL_USE_LOCAL_CPM_FILE)
  if(ANDROID)
	set(CPM_LOCAL_FILE "${CMAKE_SOURCE_DIR}/cmake/CPM_0.40.2.cmake")
  else()
	  find_file(CPM_LOCAL_FILE
		CPM_0.40.2.cmake
		PATHS ${CPM_SEARCH_PATHS} # CMake correctly expands this list.
		#PATH_SUFFIXES ${CMAKE_CURRENT_LIST_DIR}
	  )
  endif()]]

if(QATERIAL_USE_LOCAL_CPM_FILE)
  set(CPM_LOCAL_FILE "${CMAKE_SOURCE_DIR}/cmake/CPM_0.40.2.cmake")
message(STATUS "Attempting to find local CPM.cmake: ${CPM_LOCAL_FILE}")
  if(EXISTS ${CPM_LOCAL_FILE})
    message(STATUS "Using local CPM.cmake: ${CPM_LOCAL_FILE}")
    include(${CPM_LOCAL_FILE})
    return()
  endif()
endif()

#[[ Now the original from the release that can be found at
# https://github.com/cpm-cmake/CPM.cmake/releases

set(CPM_DOWNLOAD_VERSION 0.40.2)
set(CPM_HASH_SUM "c8cdc32c03816538ce22781ed72964dc864b2a34a310d3b7104812a5ca2d835d")

if(CPM_SOURCE_CACHE)
  set(CPM_DOWNLOAD_LOCATION "${CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
elseif(DEFINED ENV{CPM_SOURCE_CACHE})
  set(CPM_DOWNLOAD_LOCATION "$ENV{CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
else()
  set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
endif()

 Expand relative path. This is important if the provided path contains a tilde (~)
get_filename_component(CPM_DOWNLOAD_LOCATION ${CPM_DOWNLOAD_LOCATION} ABSOLUTE)

file(DOWNLOAD
     https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
     ${CPM_DOWNLOAD_LOCATION} EXPECTED_HASH SHA256=${CPM_HASH_SUM}
)

include(${CPM_DOWNLOAD_LOCATION})]]
