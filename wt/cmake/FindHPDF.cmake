# FindHPDF.cmake - Prefer in-tree target 'hpdf', but ensure include dirs and static lib path are exposed.
# Exports:
#   HPDF_FOUND, HPDF_INCLUDE_DIR, HPDF_INCLUDE_DIRS, HPDF_LIBRARY, HPDF_LIBRARIES, HPDF_VERSION
#
# Assumptions:
#  - You add_subdirectory(${CMAKE_SOURCE_DIR}/wt/libharu) before calling find_package(HPDF)
#  - The in-tree target is named 'hpdf' and the static archive may be named libhpdf.a

# Allow user to override hint root
if(NOT DEFINED HPDF_ROOT)
  set(HPDF_ROOT "")
endif()

# helper: known in-tree include path relative to project source
set(_IN_TREE_HP_DIR "${CMAKE_SOURCE_DIR}/wt/libharu")
set(_IN_TREE_HP_INCLUDE "${_IN_TREE_HP_DIR}/include")

# --- Prefer in-tree target 'hpdf' ---
if(TARGET hpdf)
  set(HPDF_FOUND TRUE)

  # Expose the target itself as the library variable (so consumers can link to the target)
  set(HPDF_LIBRARIES hpdf)
  set(HPDF_LIBRARY hpdf) # keep as fallback; we'll try to set to file path below if available

  # Collect include directories:
  # 1) Try the target's INTERFACE_INCLUDE_DIRECTORIES or INCLUDE_DIRECTORIES
  get_target_property(_hpdf_iface_inc hpdf INTERFACE_INCLUDE_DIRECTORIES)
  get_target_property(_hpdf_inc hpdf INCLUDE_DIRECTORIES)
  set(_collected_incs "")
  if(_hpdf_iface_inc)
    list(APPEND _collected_incs ${_hpdf_iface_inc})
  endif()
  if(_hpdf_inc)
    list(APPEND _collected_incs ${_hpdf_inc})
  endif()

  # 2) Ensure the known in-tree include dir is included (explicitly add if present)
  if(EXISTS "${_IN_TREE_HP_INCLUDE}")
    list(APPEND _collected_incs "${_IN_TREE_HP_INCLUDE}")
  endif()

  # Normalize/unique list
  list(REMOVE_DUPLICATES _collected_incs)

  if(_collected_incs)
    set(HPDF_INCLUDE_DIRS ${_collected_incs})
    # provide a single canonical include dir for older code that expects HPDF_INCLUDE_DIR
    list(GET HPDF_INCLUDE_DIRS 0 _first_hp_inc)
    set(HPDF_INCLUDE_DIR ${_first_hp_inc})
  else()
    set(HPDF_INCLUDE_DIRS "")
    set(HPDF_INCLUDE_DIR "")
  endif()

  # Try to detect a library file for libhpdf (static archive) at common locations so HPDF_LIBRARY is a path
  set(_search_paths
    "${CMAKE_BINARY_DIR}"
    "${CMAKE_BINARY_DIR}/lib"
    "${CMAKE_BINARY_DIR}/wt/libharu"
    "${CMAKE_BINARY_DIR}/wt/libharu/lib"
    "${CMAKE_SOURCE_DIR}/wt/libharu"
    "${CMAKE_SOURCE_DIR}/wt/libharu/lib"
    "${CMAKE_SOURCE_DIR}/wt/libharu/build"
    "${CMAKE_SOURCE_DIR}/wt/libharu/bin"
    "${CMAKE_INSTALL_PREFIX}/lib"
    "/usr/lib"
    "/usr/local/lib"
  )

  set(_lib_file_found "")
  foreach(_p IN LISTS _search_paths)
    if(EXISTS "${_p}/libhpdf.a")
      set(_lib_file_found "${_p}/libhpdf.a")
      break()
    elseif(EXISTS "${_p}/libhpdf.so")
      set(_lib_file_found "${_p}/libhpdf.so")
      break()
    elseif(EXISTS "${_p}/hpdf.lib") # windows
      set(_lib_file_found "${_p}/hpdf.lib")
      break()
    endif()
  endforeach()

  if(_lib_file_found)
    set(HPDF_LIBRARY "${_lib_file_found}")
    # Prefer to expose the file path in HPDF_LIBRARIES as well (consumers sometimes expect file path)
    set(HPDF_LIBRARIES "${_lib_file_found}" CACHE INTERNAL "libhpdf archive found")
  else()
    # keep HPDF_LIBRARIES as the target name (so target_link_libraries(... hpdf) still works)
    set(HPDF_LIBRARIES hpdf)
  endif()

  # Try to capture a version if the target exports one
  get_target_property(_hpdf_version hpdf VERSION)
  if(_hpdf_version AND NOT "${_hpdf_version}" STREQUAL "NOTFOUND")
    set(HPDF_VERSION ${_hpdf_version})
  endif()

  # Create alias target HPDF::HPDF for consistency with find_package style
  if(NOT TARGET HPDF::HPDF)
    add_library(HPDF::HPDF ALIAS hpdf)
  endif()

  # Cache the variables (internal to avoid user override)
  set(HPDF_FOUND TRUE CACHE INTERNAL "Found in-tree libharu (hpdf target)")
  set(HPDF_INCLUDE_DIRS "${HPDF_INCLUDE_DIRS}" CACHE INTERNAL "HPDF include dirs (in-tree)")
  set(HPDF_INCLUDE_DIR "${HPDF_INCLUDE_DIR}" CACHE INTERNAL "HPDF include dir (in-tree)")
  set(HPDF_LIBRARIES "${HPDF_LIBRARIES}" CACHE INTERNAL "HPDF libraries (in-tree)")
  if(HPDF_LIBRARY)
    set(HPDF_LIBRARY "${HPDF_LIBRARY}" CACHE INTERNAL "HPDF library (path or target)")
  endif()
  if(HPDF_VERSION)
    set(HPDF_VERSION "${HPDF_VERSION}" CACHE INTERNAL "HPDF version (in-tree)")
  endif()

  message(STATUS "FindHPDF: using in-tree 'hpdf' target; added include dirs and located library: ${HPDF_LIBRARY}")
  return()
endif()

# --- Fallback: filesystem probe (unchanged basic fallback) ---
find_path(_HPDF_INCLUDE_DIR
  NAMES hpdf.h HPDF/HPDF.h HPDF.h
  HINTS
    ${HPDF_ROOT}
    ${HARU_PREFIX}
    /usr/include
    /usr/local/include
    ${CMAKE_INSTALL_PREFIX}/include
)

find_library(_HPDF_LIBRARY
  NAMES hpdf hpdf_static libhpdf
  HINTS
    ${HPDF_ROOT}
    ${HARU_PREFIX}
    /usr/lib
    /usr/local/lib
    ${CMAKE_INSTALL_PREFIX}/lib
)

if(_HPDF_INCLUDE_DIR AND _HPDF_LIBRARY)
  set(HPDF_FOUND TRUE)
  set(HPDF_INCLUDE_DIR ${_HPDF_INCLUDE_DIR})
  set(HPDF_INCLUDE_DIRS ${_HPDF_INCLUDE_DIR})
  set(HPDF_LIBRARY ${_HPDF_LIBRARY})
  set(HPDF_LIBRARIES ${_HPDF_LIBRARY})
  set(HPDF_FOUND TRUE CACHE INTERNAL "Found system libharu")
  message(STATUS "FindHPDF: found headers at ${HPDF_INCLUDE_DIR} and library ${HPDF_LIBRARY}")
else()
  set(HPDF_FOUND FALSE)
  set(HPDF_INCLUDE_DIR "" CACHE INTERNAL "HPDF include dir (not found)")
  set(HPDF_INCLUDE_DIRS "" CACHE INTERNAL "HPDF include dirs (not found)")
  set(HPDF_LIBRARY "" CACHE INTERNAL "HPDF library (not found)")
  set(HPDF_LIBRARIES "" CACHE INTERNAL "HPDF libraries (not found)")
  set(HPDF_FOUND FALSE CACHE INTERNAL "libharu not found")
  message(STATUS "FindHPDF: libharu not found (no in-tree 'hpdf' and fallback probe failed)")
endif()

# --- If we found a system library, expose an imported target HPDF::HPDF for consistency ---
if(HPDF_FOUND AND NOT TARGET HPDF::HPDF AND HPDF_LIBRARY)
  add_library(HPDF::HPDF UNKNOWN IMPORTED)
  set_target_properties(HPDF::HPDF PROPERTIES
    IMPORTED_LOCATION "${HPDF_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${HPDF_INCLUDE_DIRS}"
  )
  set(HPDF_LIBRARIES ${HPDF_LIBRARIES} CACHE INTERNAL "HPDF libraries (imported)")
endif()
