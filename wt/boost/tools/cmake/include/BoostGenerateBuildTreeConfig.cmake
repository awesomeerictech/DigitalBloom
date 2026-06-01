# cmake/BoostGenerateBuildTreeConfig.cmake
# Generate a build-tree BoostTargets.cmake + BoostConfig.cmake from the
# export/<lib>-targets.cmake files that BoostInstall.cmake generates.
#
# Include this file AFTER your Boost build/configure (i.e. after BoostRoot / BoostInstall
# have been processed). It does not call export() — it simply consumes the per-library
# export files and produces a single BoostTargets.cmake that find_package(Boost CONFIG)
# can use.

cmake_minimum_required(VERSION 3.14)

# Where to put the generated build-tree config
set(BOOST_BUILD_CONFIG_DIR "${CMAKE_BINARY_DIR}/boost")
file(MAKE_DIRECTORY "${BOOST_BUILD_CONFIG_DIR}")

# Pattern(s) where BoostInstall wrote export files; search these directories.
# These paths match the boost build layout used in your uploaded files.
set(_search_roots
  "${CMAKE_BINARY_DIR}"             # top-level binary dir (export/*)
  "${CMAKE_BINARY_DIR}/export"      # explicit export dir
  "${CMAKE_BINARY_DIR}/boost"       # fallback
  "${CMAKE_BINARY_DIR}/boost/export"
)

# Find all export/*-targets.cmake files (per-target export fragments).
set(_export_files "")
foreach(_root IN LISTS _search_roots)
  if(EXISTS "${_root}")
    file(GLOB _found "${_root}/*-targets.cmake")
    if(_found)
      list(APPEND _export_files ${_found})
    endif()
    # also look in subdirs
    file(GLOB_RECURSE _recurse "${_root}/*-targets.cmake")
    if(_recurse)
      list(APPEND _export_files ${_recurse})
    endif()
  endif()
endforeach()

# Deduplicate & sort for deterministic output
list(REMOVE_DUPLICATES _export_files)
list(SORT _export_files)

if(NOT _export_files)
  message(WARNING "BoostGenerateBuildTreeConfig: no per-lib export files found under search roots; looking for 'export/*-targets.cmake' in ${CMAKE_BINARY_DIR}")
  # We still produce a minimal BoostTargets + BoostConfig so find_package won't totally fail:
  set(_targets_path "${BOOST_BUILD_CONFIG_DIR}/BoostTargets.cmake")
  file(WRITE "${_targets_path}" "# Generated empty BoostTargets.cmake (no export files found)\n")
  set(_config_path "${BOOST_BUILD_CONFIG_DIR}/BoostConfig.cmake")
  file(WRITE "${_config_path}" "# Generated empty BoostConfig.cmake\nset(Boost_FOUND TRUE)\n")
  list(APPEND CMAKE_PREFIX_PATH "${BOOST_BUILD_CONFIG_DIR}")
  message(STATUS "BoostGenerateBuildTreeConfig: wrote empty ${_config_path}")
  return()
endif()

# Write the combined BoostTargets.cmake
set(_targets_path "${BOOST_BUILD_CONFIG_DIR}/BoostTargets.cmake")
file(WRITE "${_targets_path}"
  "# Auto-generated BoostTargets.cmake (assembled from per-target export files)\n"
  "cmake_minimum_required(VERSION 3.14)\n\n"
)

# For each per-target export file: attempt to infer the public Boost::<name>
# from the filename and include it guarded by an if(NOT TARGET Boost::<name>) block.
foreach(_f IN LISTS _export_files)
  # Convert to cmake path (slashes normalized)
  file(TO_CMAKE_PATH "${_f}" _f_cmake)

  # try to find lib base name from filename, e.g. export/filesystem-targets.cmake -> filesystem
  get_filename_component(_fname "${_f}" NAME) # e.g. filesystem-targets.cmake
  string(REGEX REPLACE "-targets\\.cmake$" "" _lib_from_file "${_fname}")

  # Fallback: if can't find a lib name, just include the file directly (loosened guard)
  if(_lib_from_file STREQUAL "${_fname}")
    # unknown pattern: include directly
    file(APPEND "${_targets_path}" "include(\"${_f_cmake}\")\n")
  else()
    # Use a guarded include so we don't recreate targets that already exist.
    file(APPEND "${_targets_path}"
"if(NOT TARGET Boost::${_lib_from_file})\n"
"  include(\"${_f_cmake}\")\n"
"endif()\n\n"
    )
  endif()
endforeach()

message(STATUS "BoostGenerateBuildTreeConfig: wrote BoostTargets.cmake -> ${_targets_path}")

# Write the BoostConfig.cmake (simple wrapper that includes BoostTargets)
set(_config_path "${BOOST_BUILD_CONFIG_DIR}/BoostConfig.cmake")
file(WRITE "${_config_path}"
  "# Auto-generated BoostConfig.cmake (build-tree)\n"
  "cmake_minimum_required(VERSION 3.14)\n"
  "set(Boost_FOUND TRUE)\n"
  "if(NOT DEFINED Boost_ROOT)\n"
  "  set(Boost_ROOT \"${CMAKE_SOURCE_DIR}/boost\")\n"
  "endif()\n"
  "if(NOT DEFINED Boost_DIR)\n"
  "  set(Boost_DIR \"${BOOST_BUILD_CONFIG_DIR}\")\n"
  "endif()\n"
  "include(\"\${CMAKE_CURRENT_LIST_DIR}/BoostTargets.cmake\")\n"
)

# Version file so find_package(... VERSION) can behave sensibly.
set(_configver_path "${BOOST_BUILD_CONFIG_DIR}/BoostConfigVersion.cmake")
if(DEFINED BOOST_SUPERPROJECT_VERSION)
  set(_version_string "${BOOST_SUPERPROJECT_VERSION}")
else()
  # fall back to 1.0.0 if not known
  set(_version_string "1.0.0")
endif()
file(WRITE "${_configver_path}" "set(PACKAGE_VERSION \"${_version_string}\")\n")

message(STATUS "BoostGenerateBuildTreeConfig: wrote BoostConfig.cmake -> ${_config_path}")
message(STATUS "BoostGenerateBuildTreeConfig: wrote BoostConfigVersion.cmake -> ${_configver_path}")

# Make the generated config discoverable in the same configure by appending to CMAKE_PREFIX_PATH
list(APPEND CMAKE_PREFIX_PATH "${BOOST_BUILD_CONFIG_DIR}")
message(STATUS "BoostGenerateBuildTreeConfig: appended ${BOOST_BUILD_CONFIG_DIR} to CMAKE_PREFIX_PATH")
