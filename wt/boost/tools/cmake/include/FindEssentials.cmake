# FindEssentials.cmake
#
# Register in-tree copies of xz/zstd/zlib so libarchive's FIND_PACKAGE
# logic will accept them. Put this BEFORE add_subdirectory(libarchive).
#

# --- helpers ---------------------------------------------------------------
function(_maybe_add_subdir rel_or_abs_path)
  if(NOT IS_ABSOLUTE "${rel_or_abs_path}")
    file(TO_CMAKE_PATH "${CMAKE_SOURCE_DIR}/${rel_or_abs_path}" _candidate)
  else()
    file(TO_CMAKE_PATH "${rel_or_abs_path}" _candidate)
  endif()

  if(EXISTS "${_candidate}/CMakeLists.txt")
    message(STATUS "FindEssentials: adding subdir ${_candidate}")
    add_subdirectory("${_candidate}")
  else()
    message(STATUS "FindEssentials: skipping missing subdir ${_candidate}")
  endif()
endfunction()

function(_get_target_include target out_var)
  set(_inc "")
  if(TARGET ${target})
    get_target_property(_inc ${target} INTERFACE_INCLUDE_DIRECTORIES)
    if(NOT _inc OR _inc STREQUAL "NOTFOUND")
      get_target_property(_inc ${target} INCLUDE_DIRECTORIES)
    endif()
  endif()
  if(_inc AND NOT _inc STREQUAL "NOTFOUND")
    separate_arguments(_inc)
    list(JOIN _inc ";" _inc_joined)
    set(${out_var} "${_inc_joined}" PARENT_SCOPE)
  else()
    set(${out_var} "" PARENT_SCOPE)
  endif()
endfunction()

# --- add the subprojects we build in-tree (only if present) -----------------
_maybe_add_subdir("${CMAKE_SOURCE_DIR}/wt/boost/libarchive/xz")
_maybe_add_subdir("${CMAKE_SOURCE_DIR}/wt/boost/libarchive/zstd")
# try zlib too; helper will skip if missing
_maybe_add_subdir("${CMAKE_SOURCE_DIR}/wt/boost/libarchive/zlib")

# --- liblzma (xz) ----------------------------------------------------------
if(TARGET liblzma)
  _get_target_include(liblzma _liblzma_inc)

  if(NOT DEFINED LIBLZMA_LIBRARIES)
    set(LIBLZMA_LIBRARIES liblzma CACHE STRING "liblzma library (in-tree target)")
  endif()
  if(NOT DEFINED LIBLZMA_FOUND)
    set(LIBLZMA_FOUND TRUE CACHE BOOL "Found liblzma (in-tree)")
  endif()
  if(NOT DEFINED LIBLZMA_INCLUDE_DIR)
    set(LIBLZMA_INCLUDE_DIR "${_liblzma_inc}" CACHE PATH "liblzma include dir (in-tree)")
  endif()
  if(NOT DEFINED LIBLZMA_INCLUDE_DIRS)
    set(LIBLZMA_INCLUDE_DIRS "${_liblzma_inc}" CACHE PATH "liblzma include dirs (in-tree)")
  endif()

  if(NOT DEFINED LIBLZMA_LIBRARY_FILE)
    set(LIBLZMA_LIBRARY_FILE "$<TARGET_FILE:liblzma>" CACHE STRING "liblzma library file (generator expr -> liblzma)")
  endif()
endif()

# --- zstd (libzstd_shared / libzstd_static) ---------------------------------
if(TARGET libzstd_shared)
  _get_target_include(libzstd_shared _zstd_inc)

  if(NOT DEFINED ZSTD_FOUND)
    set(ZSTD_FOUND TRUE CACHE BOOL "Found zstd (in-tree shared)")
  endif()
  if(NOT DEFINED ZSTD_INCLUDE_DIR)
    set(ZSTD_INCLUDE_DIR "${_zstd_inc}" CACHE PATH "zstd include dir (in-tree)")
  endif()
  if(NOT DEFINED ZSTD_LIBRARY)
    set(ZSTD_LIBRARY libzstd_shared CACHE STRING "zstd library (in-tree shared target)")
  endif()
  if(NOT DEFINED ZSTD_LIBRARY_FILE)
    set(ZSTD_LIBRARY_FILE "$<TARGET_FILE:libzstd_shared>" CACHE STRING "zstd library file (generator expr -> libzstd_shared)")
  endif()

elseif(TARGET libzstd_static)
  _get_target_include(libzstd_static _zstd_inc)

  if(NOT DEFINED ZSTD_FOUND)
    set(ZSTD_FOUND TRUE CACHE BOOL "Found zstd (in-tree static)")
  endif()
  if(NOT DEFINED ZSTD_INCLUDE_DIR)
    set(ZSTD_INCLUDE_DIR "${_zstd_inc}" CACHE PATH "zstd include dir (in-tree)")
  endif()
  if(NOT DEFINED ZSTD_LIBRARY)
    set(ZSTD_LIBRARY libzstd_static CACHE STRING "zstd library (in-tree static target)")
  endif()
  if(NOT DEFINED ZSTD_LIBRARY_FILE)
    set(ZSTD_LIBRARY_FILE "$<TARGET_FILE:libzstd_static>" CACHE STRING "zstd library file (generator expr -> libzstd_static)")
  endif()
endif()

# --- zlib (zlib / zlibstatic) ------------------------------------------------
if(TARGET zlib OR TARGET zlibstatic)
  if(TARGET zlib)
    _get_target_include(zlib _zlib_inc)
    set(_zlib_target_name zlib)
  else()
    _get_target_include(zlibstatic _zlib_inc)
    set(_zlib_target_name zlibstatic)
  endif()

  if(NOT DEFINED ZLIB_FOUND)
    set(ZLIB_FOUND TRUE CACHE BOOL "Found zlib (in-tree)")
  endif()
  if(NOT DEFINED ZLIB_INCLUDE_DIR)
    set(ZLIB_INCLUDE_DIR "${_zlib_inc}" CACHE PATH "zlib include dir (in-tree)")
  endif()

  # modern target-name form for consumers
  if(NOT DEFINED ZLIB_LIBRARIES)
    set(ZLIB_LIBRARIES "${_zlib_target_name}" CACHE STRING "zlib library (in-tree target name)")
  endif()

  # provide path-like generator-expr too for old find-style consumers
  if(NOT DEFINED ZLIB_LIBRARY)
    set(ZLIB_LIBRARY "$<TARGET_FILE:${_zlib_target_name}>" CACHE STRING "zlib library file (generator expr -> ${_zlib_target_name})")
  endif()
  if(NOT DEFINED ZLIB_LIBRARY_FILE)
    set(ZLIB_LIBRARY_FILE "$<TARGET_FILE:${_zlib_target_name}>" CACHE STRING "zlib library file (generator expr -> ${_zlib_target_name})")
  endif()

  message(STATUS "FindEssentials: using in-tree zlib target '${_zlib_target_name}'")
  message(STATUS "FindEssentials: ZLIB_LIBRARIES=${ZLIB_LIBRARIES}")
  message(STATUS "FindEssentials: ZLIB_LIBRARY=${ZLIB_LIBRARY}")

else()
  # No in-tree zlib target — try to locate headers in repository source or build tree
  set(_zlib_hints
    "${CMAKE_SOURCE_DIR}/wt/boost/libarchive/zlib"
    "${CMAKE_SOURCE_DIR}/wt/libharu"
    "${CMAKE_BINARY_DIR}/wt/boost/libarchive/zlib"
    "${CMAKE_BINARY_DIR}/wt/libharu"
  )

  find_path(_zlib_header_dir NAMES zlib.h HINTS ${_zlib_hints} PATH_SUFFIXES include include/zlib)
  if(_zlib_header_dir)
    if(NOT DEFINED ZLIB_FOUND)
      set(ZLIB_FOUND TRUE CACHE BOOL "Found zlib (headers present)")
    endif()
    if(NOT DEFINED ZLIB_INCLUDE_DIR)
      set(ZLIB_INCLUDE_DIR "${_zlib_header_dir}" CACHE PATH "zlib include dir (detected)")
    endif()

    if(NOT TARGET ZLIB::ZLIB)
      add_library(ZLIB::ZLIB UNKNOWN IMPORTED)
      set_target_properties(ZLIB::ZLIB PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${_zlib_header_dir}"
      )

      foreach(_lib_candidate
          "${CMAKE_BINARY_DIR}/wt/boost/libarchive/zlib/libz.a"
          "${CMAKE_BINARY_DIR}/wt/boost/libarchive/zlib/zlib.lib"
          "${CMAKE_BINARY_DIR}/wt/boost/libarchive/zlib/libz.so"
          "${CMAKE_BINARY_DIR}/wt/boost/libarchive/zlib/libz.dylib"
          )
        if(EXISTS "${_lib_candidate}")
          set_target_properties(ZLIB::ZLIB PROPERTIES IMPORTED_LOCATION "${_lib_candidate}")
          set(ZLIB_LIBRARY_FILE "${_lib_candidate}" CACHE PATH "zlib library file (detected)")
          break()
        endif()
      endforeach()
    endif()

    if(NOT DEFINED ZLIB_LIBRARIES)
      set(ZLIB_LIBRARIES "ZLIB::ZLIB" CACHE STRING "zlib library (IMPORTED target)")
    endif()
  endif()
endif()

# --- debug output (compact) ------------------------------------------------
message(STATUS "FindEssentials: LIBLZMA_FOUND=${LIBLZMA_FOUND}")
message(STATUS "FindEssentials: ZSTD_FOUND=${ZSTD_FOUND}")
message(STATUS "FindEssentials: ZLIB_FOUND=${ZLIB_FOUND}")

if(DEFINED LIBLZMA_INCLUDE_DIRS)
  message(STATUS "FindEssentials: LIBLZMA_INCLUDE_DIRS=${LIBLZMA_INCLUDE_DIRS}")
endif()
if(DEFINED ZSTD_INCLUDE_DIR)
  message(STATUS "FindEssentials: ZSTD_INCLUDE_DIR=${ZSTD_INCLUDE_DIR}")
endif()
if(DEFINED ZLIB_INCLUDE_DIR)
  message(STATUS "FindEssentials: ZLIB_INCLUDE_DIR=${ZLIB_INCLUDE_DIR}")
endif()

# --------------------------------------------------------------------------
# Create consistent consumer-facing targets / aliases:
# - ZLIB::ZLIB  -> prefer zlib, fallback zlibstatic, fallback IMPORTED
# - ZSTD::ZSTD  -> prefer libzstd (interface), fallback libzstd_shared/libzstd_static, fallback IMPORTED
# - liblzma::liblzma and LibLZMA::LibLZMA -> alias liblzma when present,
#   or create IMPORTED fallback
# --------------------------------------------------------------------------

# --- ZLIB::ZLIB alias or imported fallback -------------------------------
if(TARGET zlib AND NOT TARGET "ZLIB::ZLIB")
  add_library("ZLIB::ZLIB" ALIAS zlib)
  message(STATUS "FindEssentials: created alias ZLIB::ZLIB -> zlib")
elseif(TARGET zlibstatic AND NOT TARGET "ZLIB::ZLIB")
  add_library("ZLIB::ZLIB" ALIAS zlibstatic)
  message(STATUS "FindEssentials: created alias ZLIB::ZLIB -> zlibstatic")
elseif(NOT TARGET "ZLIB::ZLIB")
  if(DEFINED ZLIB_INCLUDE_DIR OR DEFINED ZLIB_LIBRARY_FILE OR DEFINED ZLIB_LIBRARY)
    add_library("ZLIB::ZLIB" UNKNOWN IMPORTED)
    if(DEFINED ZLIB_INCLUDE_DIR)
      set_target_properties("ZLIB::ZLIB" PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${ZLIB_INCLUDE_DIR}")
    endif()
    if(DEFINED ZLIB_LIBRARY_FILE)
      set_target_properties("ZLIB::ZLIB" PROPERTIES IMPORTED_LOCATION "${ZLIB_LIBRARY_FILE}")
    elseif(DEFINED ZLIB_LIBRARY)
      set_target_properties("ZLIB::ZLIB" PROPERTIES IMPORTED_LOCATION "${ZLIB_LIBRARY}")
    endif()
    if(NOT DEFINED ZLIB_LIBRARIES)
      set(ZLIB_LIBRARIES "ZLIB::ZLIB" CACHE STRING "zlib library (IMPORTED fallback)")
    endif()
    if(NOT DEFINED ZLIB_FOUND)
      set(ZLIB_FOUND TRUE CACHE BOOL "Found zlib (IMPORTED fallback)")
    endif()
    message(STATUS "FindEssentials: created IMPORTED target ZLIB::ZLIB (fallback)")
  endif()
endif()

if(NOT DEFINED ZLIB_LIBRARIES AND TARGET "ZLIB::ZLIB")
  set(ZLIB_LIBRARIES "ZLIB::ZLIB" CACHE STRING "zlib library (target form)")
endif()
if(NOT DEFINED ZLIB_LIBRARY AND TARGET "ZLIB::ZLIB")
  set(ZLIB_LIBRARY "$<TARGET_FILE:ZLIB::ZLIB>" CACHE STRING "zlib library file (generator expr -> ZLIB::ZLIB)")
  set(ZLIB_LIBRARY_FILE "${ZLIB_LIBRARY}" CACHE STRING "zlib library file (generator expr -> ZLIB::ZLIB)")
endif()

# --- ZSTD::ZSTD alias or imported fallback -------------------------------
if(TARGET libzstd AND NOT TARGET "ZSTD::ZSTD")
  add_library("ZSTD::ZSTD" ALIAS libzstd)
  message(STATUS "FindEssentials: created alias ZSTD::ZSTD -> libzstd")
elseif(TARGET libzstd_shared AND NOT TARGET "ZSTD::ZSTD")
  add_library("ZSTD::ZSTD" ALIAS libzstd_shared)
  message(STATUS "FindEssentials: created alias ZSTD::ZSTD -> libzstd_shared")
elseif(TARGET libzstd_static AND NOT TARGET "ZSTD::ZSTD")
  add_library("ZSTD::ZSTD" ALIAS libzstd_static)
  message(STATUS "FindEssentials: created alias ZSTD::ZSTD -> libzstd_static")
elseif(NOT TARGET "ZSTD::ZSTD")
  if(DEFINED ZSTD_INCLUDE_DIR OR DEFINED ZSTD_LIBRARY_FILE OR DEFINED ZSTD_LIBRARY)
    add_library("ZSTD::ZSTD" UNKNOWN IMPORTED)
    if(DEFINED ZSTD_INCLUDE_DIR)
      set_target_properties("ZSTD::ZSTD" PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${ZSTD_INCLUDE_DIR}")
    endif()
    if(DEFINED ZSTD_LIBRARY_FILE)
      set_target_properties("ZSTD::ZSTD" PROPERTIES IMPORTED_LOCATION "${ZSTD_LIBRARY_FILE}")
    elseif(DEFINED ZSTD_LIBRARY)
      set_target_properties("ZSTD::ZSTD" PROPERTIES IMPORTED_LOCATION "${ZSTD_LIBRARY}")
    endif()
    if(NOT DEFINED ZSTD_LIBRARIES)
      set(ZSTD_LIBRARIES "ZSTD::ZSTD" CACHE STRING "zstd library (IMPORTED fallback)")
    endif()
    if(NOT DEFINED Zstd_FOUND)
      set(Zstd_FOUND TRUE CACHE BOOL "Found zstd (IMPORTED fallback)")
    endif()
    message(STATUS "FindEssentials: created IMPORTED target ZSTD::ZSTD (fallback)")
  endif()
endif()

if(NOT DEFINED ZSTD_LIBRARY_FILE AND TARGET "ZSTD::ZSTD")
  set(ZSTD_LIBRARY_FILE "$<TARGET_FILE:ZSTD::ZSTD>" CACHE STRING "zstd library file (generator expr -> ZSTD::ZSTD)")
endif()
if(NOT DEFINED ZSTD_LIBRARY AND TARGET "ZSTD::ZSTD")
  set(ZSTD_LIBRARY "$<TARGET_FILE:ZSTD::ZSTD>" CACHE STRING "zstd library file (generator expr -> ZSTD::ZSTD)")
endif()

# --- liblzma aliases (liblzma / LibLZMA::LibLZMA) --------------------------
if(TARGET liblzma AND NOT TARGET "liblzma::liblzma")
  add_library("liblzma::liblzma" ALIAS liblzma)
  message(STATUS "FindEssentials: created alias liblzma::liblzma -> liblzma")
endif()

if(TARGET liblzma AND NOT TARGET "LibLZMA::LibLZMA")
  add_library("LibLZMA::LibLZMA" ALIAS liblzma)
  message(STATUS "FindEssentials: created alias LibLZMA::LibLZMA -> liblzma")
endif()

if(NOT TARGET "liblzma::liblzma" AND (DEFINED LIBLZMA_INCLUDE_DIR OR DEFINED LIBLZMA_LIBRARIES OR DEFINED LIBLZMA_LIBRARY))
  add_library("liblzma::liblzma" UNKNOWN IMPORTED)
  if(DEFINED LIBLZMA_INCLUDE_DIR)
    set_target_properties("liblzma::liblzma" PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${LIBLZMA_INCLUDE_DIR}")
  endif()
  if(DEFINED LIBLZMA_LIBRARIES)
    set_target_properties("liblzma::liblzma" PROPERTIES IMPORTED_LOCATION "${LIBLZMA_LIBRARIES}")
  endif()
  message(STATUS "FindEssentials: created IMPORTED target liblzma::liblzma (fallback)")
endif()

if(NOT TARGET "LibLZMA::LibLZMA" AND TARGET "liblzma::liblzma")
  add_library("LibLZMA::LibLZMA" ALIAS "liblzma::liblzma")
endif()

# Final debug summary
message(STATUS "FindEssentials: detection summary -> LIBLZMA_FOUND=${LIBLZMA_FOUND}; ZSTD_FOUND=${ZSTD_FOUND}; ZLIB_FOUND=${ZLIB_FOUND}")
