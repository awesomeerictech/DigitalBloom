@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ SAMPLE ONE @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@


# WtFindBoost.txt - comprehensive in-tree Boost detection and include discovery
#
# Behavior:
#  - If Boost is in-tree (Boost:: targets exist), map targets into Wt variables and
#    collect include directories for modules listed in Boost_COMPONENTS plus any
#    entries in BOOST_INCLUDE_LIBRARIES (if present).
#  - For each module scanned:
#      * add any <module>/include directories (any depth)
#      * add immediate child submodule include dirs (e.g. numeric/ublas/include)
#      * add immediate child submodule dir if it directly contains headers
#      * for headers with no 'include' ancestor, add the header's parent directory
#  - Deduplicate include dirs and set BOOST_INCLUDE_DIRS.
#  - Otherwise fall back to find_package(Boost ...).
#

# default state
SET(BOOST_WT_FOUND FALSE)
SET(BOOST_WT_MT_FOUND FALSE)
SET(BOOST_WTHTTP_FOUND FALSE)
SET(BOOST_WTHTTP_MT_FOUND FALSE)
SET(BOOST_SUPPORT_LIBRARIES "")

IF(NOT DEFINED Boost_ADDITIONAL_VERSIONS)
  SET(Boost_ADDITIONAL_VERSIONS
    "1.66" "1.67" "1.68" "1.69" "1.70" "1.71" "1.72" "1.73" "1.74" "1.75"
    "1.76" "1.77" "1.78" "1.79" "1.80" "1.81" "1.82" "1.83" "1.84" "1.85"
    "1.86" "1.87" "1.88")
ENDIF()

SET(Boost_USE_MULTITHREADED ON)

IF(DEFINED BOOST_PREFIX AND NOT DEFINED BOOST_ROOT)
  SET(BOOST_ROOT ${BOOST_PREFIX})
ENDIF()

IF(WIN32)
  IF(BOOST_DYNAMIC)
    SET(Boost_USE_STATIC_LIBS OFF)
  ELSE()
    SET(Boost_USE_STATIC_LIBS ON)
  ENDIF()
ENDIF()

SET(Boost_DEBUG ON)

# Components Wt may use (adjust as needed)
SET(Boost_COMPONENTS
    program_options
    thread
    asio
    filesystem
    chrono
    system
    numeric
)

IF(WT_CPP17_FILESYSTEM_IMPLEMENTATION STREQUAL "boost")
  LIST(APPEND Boost_COMPONENTS filesystem)
ENDIF()

IF(WIN32 AND BOOST_DYNAMIC AND BUILD_TESTS)
  LIST(APPEND Boost_COMPONENTS unit_test_framework)
ENDIF()

#
# --- Detect in-tree Boost targets created by add_subdirectory(boost) ---
#
set(_wt_in_tree_boost FALSE)
set(_wt_boost_includes "")
set(_wt_boost_collected_libs "")

foreach(_comp ${Boost_COMPONENTS})
  if(TARGET Boost::${_comp})
    set(_wt_in_tree_boost TRUE)
    # collect target reference
    list(APPEND _wt_boost_collected_libs Boost::${_comp})
    # try to capture include dirs from first target that provides them
    if(NOT _wt_boost_includes)
      get_target_property(_incdirs Boost::${_comp} INTERFACE_INCLUDE_DIRECTORIES)
      if(_incdirs)
        set(_wt_boost_includes "${_incdirs}")
      endif()
    endif()
  endif()
endforeach()

if(_wt_in_tree_boost)
  message(STATUS "WtFindBoost: using in-tree Boost targets (Boost::...)")

  set(Boost_FOUND TRUE CACHE INTERNAL "Found in-tree Boost")
  set(BOOST_LIB_DIRS "")

  # Map commonly-used Wt libs (thread, system, filesystem) and others if present
  set(_wt_libs "")
  if(TARGET Boost::thread)
    list(APPEND _wt_libs Boost::thread)
    set(BOOST_THREAD_LIB Boost::thread)
  endif()
  if(TARGET Boost::system)
    list(APPEND _wt_libs Boost::system)
    set(BOOST_SYSTEM_LIB Boost::system)
  endif()
  if(TARGET Boost::filesystem)
    list(APPEND _wt_libs Boost::filesystem)
    set(BOOST_FS_LIB Boost::filesystem)
  endif()
  if(TARGET Boost::asio)
    list(APPEND _wt_libs Boost::asio)
    set(BOOST_ASIO_LIB Boost::asio)
  endif()
  if(TARGET Boost::chrono)
    list(APPEND _wt_libs Boost::chrono)
    set(BOOST_CHRONO_LIB Boost::chrono)
  endif()
  if(TARGET Boost::numeric)
    list(APPEND _wt_libs Boost::numeric)
    set(BOOST_NUMERIC_LIB Boost::numeric)
  endif()

  set(BOOST_WT_LIBRARIES ${_wt_libs})
  set(BOOST_WTHTTP_LIBRARIES ${_wt_libs})

  if(TARGET Boost::headers)
    list(APPEND BOOST_WT_LIBRARIES Boost::headers)
    list(APPEND BOOST_WTHTTP_LIBRARIES Boost::headers)
  endif()

  set(BOOST_WT_FOUND TRUE)
  set(BOOST_WT_MT_FOUND TRUE)
  set(BOOST_WTHTTP_FOUND TRUE)
  set(BOOST_WTHTTP_MT_FOUND TRUE)

  #
  # --- Per-component exhaustive include collection ---
  #
  set(_collected_includes "")

  # 1) preserve any INTERFACE include dirs we discovered earlier
  if(_wt_boost_includes)
    list(APPEND _collected_includes ${_wt_boost_includes})
  endif()

  # 2) include dirs from Boost::headers umbrella target (if present)
  if(TARGET Boost::headers)
    get_target_property(_hdrincs Boost::headers INTERFACE_INCLUDE_DIRECTORIES)
    if(_hdrincs)
      list(APPEND _collected_includes ${_hdrincs})
    endif()
  endif()

  # 3) Build module list to scan: BOOST_INCLUDE_LIBRARIES (cache) + Boost_COMPONENTS
  set(_mods_to_scan "")
  if(DEFINED BOOST_INCLUDE_LIBRARIES)
    list(APPEND _mods_to_scan ${BOOST_INCLUDE_LIBRARIES})
  endif()
  list(APPEND _mods_to_scan ${Boost_COMPONENTS})
  list(REMOVE_DUPLICATES _mods_to_scan)

  # 4) For each module, scan only that module tree
  foreach(_mod IN LISTS _mods_to_scan)
    string(STRIP "${_mod}" _mod_trim)
    if(NOT _mod_trim)
      continue()
    endif()

    set(_moddir "${CMAKE_SOURCE_DIR}/boost/libs/${_mod_trim}")
    if(NOT EXISTS "${_moddir}")
      continue()
    endif()

    # 4a) Add a direct include folder under the module if present
    if(EXISTS "${_moddir}/include" AND IS_DIRECTORY "${_moddir}/include")
      list(APPEND _collected_includes "${_moddir}/include")
    endif()

    # 4b) Collect any nested include/ directories under the module (e.g. numeric/ublas/include)
    file(GLOB_RECURSE _inc_dirs RELATIVE "${_moddir}" "${_moddir}/**/include")
    foreach(_rel IN LISTS _inc_dirs)
      set(_abs "${_moddir}/${_rel}")
      if(EXISTS "${_abs}" AND IS_DIRECTORY "${_abs}")
        list(APPEND _collected_includes "${_abs}")
      endif()
    endforeach()

    # 4c) Collect header files and add nearest 'include' ancestor or header's dir
    file(GLOB_RECURSE _hdrs RELATIVE "${_moddir}"
         "${_moddir}/**/*.hpp" "${_moddir}/**/*.h" "${_moddir}/**/*.ipp" "${_moddir}/**/*.inl")
    foreach(_hdr IN LISTS _hdrs)
      set(_hdr_full "${_moddir}/${_hdr}")
      get_filename_component(_hdr_dir "${_hdr_full}" DIRECTORY)
      if(NOT _hdr_dir)
        continue()
      endif()

      # climb upward from _hdr_dir seeking 'include' ancestor
      set(_search "${_hdr_dir}")
      set(_found_include_dir "")
      while(TRUE)
        get_filename_component(_name "${_search}" NAME)
        if(_name STREQUAL "include")
          set(_found_include_dir "${_search}")
          break()
        endif()
        get_filename_component(_parent "${_search}" DIRECTORY)
        if(_parent STREQUAL _search OR NOT _parent)
          break()
        endif()
        set(_search "${_parent}")
      endwhile()

      if(_found_include_dir)
        list(APPEND _collected_includes "${_found_include_dir}")
      else()
        list(APPEND _collected_includes "${_hdr_dir}")
      endif()
    endforeach()

    # 4d) Ensure immediate child subfolders that contain headers are added.
    # This covers layouts like boost/libs/numeric/ublas/*.hpp (no include/ dir)
    file(GLOB _child_dirs RELATIVE "${_moddir}" "${_moddir}/*")
    foreach(_child IN LISTS _child_dirs)
      set(_child_abs "${_moddir}/${_child}")
      if(IS_DIRECTORY "${_child_abs}")
        # if child has its own include dir, add it
        if(EXISTS "${_child_abs}/include" AND IS_DIRECTORY "${_child_abs}/include")
          list(APPEND _collected_includes "${_child_abs}/include")
        endif()
        # or if the child directory directly contains headers, add the child dir
        file(GLOB _child_hdrs "${_child_abs}/*.hpp" "${_child_abs}/*.h" "${_child_abs}/*.ipp" "${_child_abs}/*.inl")
        if(_child_hdrs)
          list(APPEND _collected_includes "${_child_abs}")
        else()
          # also check deeper for header files within child (sub-subfolders)
          file(GLOB_RECURSE _child_hdrs2 RELATIVE "${_child_abs}" "${_child_abs}/**/*.hpp" "${_child_abs}/**/*.h")
          if(_child_hdrs2)
            list(APPEND _collected_includes "${_child_abs}")
          endif()
        endif()
      endif()
    endforeach()

  endforeach()

  # 5) final fallback: top-level boost dir
  list(APPEND _collected_includes "${CMAKE_SOURCE_DIR}/boost")

  # 6) deduplicate and set the variable Wt expects
  list(REMOVE_DUPLICATES _collected_includes)
  set(BOOST_INCLUDE_DIRS ${_collected_includes})
  # small debug summary to help verification
  list(LENGTH _collected_includes _ci_len)
  message(STATUS "WtFindBoost: collected ${_ci_len} Boost include directories (first few):")
  foreach(_i RANGE 0 4)
    if(_i LESS _ci_len)
      list(GET _collected_includes ${_i} _t)
      message(STATUS "  ${_t}")
    endif()
  endforeach()

  # cleanup temp variables
  unset(_wt_boost_includes)
  unset(_wt_boost_collected_libs)
  unset(_wt_in_tree_boost)
  unset(_wt_libs)
  unset(_collected_includes)
  unset(_mods_to_scan)
  unset(_inc_dirs)
  unset(_hdrs)
  unset(_child_dirs)
  unset(_child_hdrs)
  unset(_child_hdrs2)

else()
  #
  # --- No in-tree targets — fall back to CMake's find_package(Boost) ---
  #
  function(find_boost_system)
    find_package(Boost 1.66 QUIET COMPONENTS system)
    set(_BOOST_FOUND ${Boost_FOUND} PARENT_SCOPE)
    if(DEFINED Boost_VERSION_MAJOR)
      set(_BOOST_MAJOR_VERSION ${Boost_VERSION_MAJOR} PARENT_SCOPE)
      set(_BOOST_MINOR_VERSION ${Boost_VERSION_MINOR} PARENT_SCOPE)
    else()
      set(_BOOST_MAJOR_VERSION ${Boost_MAJOR_VERSION} PARENT_SCOPE)
      set(_BOOST_MINOR_VERSION ${Boost_MINOR_VERSION} PARENT_SCOPE)
    endif()
  endfunction()

  find_boost_system()

  if(_BOOST_FOUND AND _BOOST_MAJOR_VERSION EQUAL 1 AND _BOOST_MINOR_VERSION LESS 69)
    LIST(APPEND Boost_COMPONENTS system)
  endif()

  # clear temporaries
  set(_BOOST_FOUND)
  set(_BOOST_MAJOR_VERSION)
  set(_BOOST_MINOR_VERSION)

  FIND_PACKAGE(Boost 1.66 REQUIRED COMPONENTS ${Boost_COMPONENTS})

  # If find_package provided IMPORTED targets, map them to Wt vars
  foreach(_target ${Boost_COMPONENTS})
    if(TARGET Boost::${_target})
      string(TOUPPER ${_target} _TARGET_UP)
      set(Boost_${_TARGET_UP}_LIBRARY Boost::${_target})
    endif()
  endforeach()

  if(Boost_FOUND)
    if(Boost_VERSION_MAJOR AND NOT Boost_MAJOR_VERSION)
      set(Boost_MAJOR_VERSION ${Boost_VERSION_MAJOR})
    endif()
    if(Boost_VERSION_MINOR AND NOT Boost_MINOR_VERSION)
      set(Boost_MINOR_VERSION ${Boost_VERSION_MINOR})
    endif()

    # Translate official CMake vars to Wt expected names (safe guards)
    if(DEFINED Boost_INCLUDE_DIRS)
      set(BOOST_INCLUDE_DIRS ${Boost_INCLUDE_DIRS})
    endif()
    if(DEFINED Boost_LIBRARY_DIRS)
      set(BOOST_LIB_DIRS ${Boost_LIBRARY_DIRS})
    endif()
    if(DEFINED Boost_FILESYSTEM_LIBRARY)
      set(BOOST_FS_LIB ${Boost_FILESYSTEM_LIBRARY})
    endif()
    if(DEFINED Boost_PROGRAM_OPTIONS_LIBRARY)
      set(BOOST_PO_LIB ${Boost_PROGRAM_OPTIONS_LIBRARY})
    endif()
    if(DEFINED Boost_SYSTEM_LIBRARY)
      set(BOOST_SYSTEM_LIB ${Boost_SYSTEM_LIBRARY})
    endif()
    if(DEFINED Boost_THREAD_LIBRARY)
      set(BOOST_THREAD_LIB ${Boost_THREAD_LIBRARY})
    endif()

    if(WIN32 AND BOOST_DYNAMIC AND BUILD_TESTS)
      set(BOOST_UT_LIB ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
    else()
      set(BOOST_UT_LIB "")
    endif()

    set(BOOST_WT_MT_FOUND TRUE)
    set(BOOST_WT_FOUND TRUE)
    set(BOOST_WTHTTP_MT_FOUND TRUE)
    set(BOOST_WTHTTP_FOUND TRUE)

    if(MSVC AND Boost_MAJOR_VERSION EQUAL 1 AND Boost_MINOR_VERSION LESS 70)
      set(BOOST_WT_LIBRARIES "")
      set(BOOST_WTHTTP_LIBRARIES "")
      set(BOOST_TEST_LIBRARIES "")
    else()
      set(BOOST_WT_LIBRARIES
        ${Boost_THREAD_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_FILESYSTEM_LIBRARY}
      )
      set(BOOST_WTHTTP_LIBRARIES
        ${Boost_THREAD_LIBRARY}
        ${Boost_PROGRAM_OPTIONS_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_FILESYSTEM_LIBRARY}
      )
      set(BOOST_TEST_LIBRARIES ${BOOST_UT_LIB})
    endif()

    if(TARGET Boost::headers)
      set(BOOST_WT_LIBRARIES ${BOOST_WT_LIBRARIES} Boost::headers)
      set(BOOST_WTHTTP_LIBRARIES ${BOOST_WTHTTP_LIBRARIES} Boost::headers)
    endif()
  endif()
endif()


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ ORIGINAL @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@


# WtFindBoost.txt - filesystem-first in-tree Boost detection and exhaustive include discovery
#
# Behavior:
#  - If Boost is in-tree (Boost:: targets exist), perform a filesystem-first scan for every
#    module in Boost_COMPONENTS plus any entries in BOOST_INCLUDE_LIBRARIES (if present).
#  - Discover and add absolute include directories such as:
#      boost/libs/<module>/include
#      boost/libs/<module>/<submodule>/include
#      boost/libs/<module>/<submodule>   (if headers are directly under the submodule)
#    and also add header-parent directories where necessary.
#  - Intentionally ignores generator expressions from Boost targets so only real
#    absolute, existing directories are produced.
#  - Falls back to find_package(Boost ...) when no in-tree Boost targets are available.
#

# default state
set(BOOST_WT_FOUND FALSE)
set(BOOST_WT_MT_FOUND FALSE)
set(BOOST_WTHTTP_FOUND FALSE)
set(BOOST_WTHTTP_MT_FOUND FALSE)
set(BOOST_SUPPORT_LIBRARIES "")

# Recommended Boost components Wt may use — adjust if needed.
set(Boost_COMPONENTS
  program_options
  thread
  asio
  filesystem
  chrono
  system
  numeric
)

if(WT_CPP17_FILESYSTEM_IMPLEMENTATION STREQUAL "boost")
  list(APPEND Boost_COMPONENTS filesystem)
endif()

# detect in-tree Boost (presence of imported targets Boost::<component>)
set(_wt_in_tree_boost FALSE)
foreach(_comp ${Boost_COMPONENTS})
  if(TARGET Boost::${_comp})
    set(_wt_in_tree_boost TRUE)
  endif()
endforeach()

if(_wt_in_tree_boost)
  message(STATUS "WtFindBoost: using in-tree Boost targets (Boost::...)")
  set(Boost_FOUND TRUE CACHE INTERNAL "Found in-tree Boost")
  set(BOOST_LIB_DIRS "")

  # Map Boost::XXX targets into Wt variables (but do NOT use their INTERFACE_INCLUDE_DIRECTORIES
  # since those may contain generator expressions).
  set(_wt_libs "")
  foreach(_lib thread system filesystem asio chrono numeric program_options)
    if(TARGET Boost::${_lib})
      list(APPEND _wt_libs Boost::${_lib})
      string(TOUPPER ${_lib} _ULIB)
      set(BOOST_${_ULIB}_LIB Boost::${_lib})
    endif()
  endforeach()

  set(BOOST_WT_LIBRARIES ${_wt_libs})
  set(BOOST_WTHTTP_LIBRARIES ${_wt_libs})

  if(TARGET Boost::headers)
    list(APPEND BOOST_WT_LIBRARIES Boost::headers)
    list(APPEND BOOST_WTHTTP_LIBRARIES Boost::headers)
  endif()

  # --------------------------------------------------------------------------
  # Filesystem-first include discovery
  # --------------------------------------------------------------------------
  set(_collected_includes "")

  # Build list of modules to scan: BOOST_INCLUDE_LIBRARIES (if provided) + Boost_COMPONENTS
  set(_mods "")
  if(DEFINED BOOST_INCLUDE_LIBRARIES)
    list(APPEND _mods ${BOOST_INCLUDE_LIBRARIES})
  endif()
  list(APPEND _mods ${Boost_COMPONENTS})
  list(REMOVE_DUPLICATES _mods)

  # For each module, do absolute-path scans for include directories and headers
  foreach(_mod IN LISTS _mods)
    string(STRIP "${_mod}" _m)
    if(NOT _m)
      continue()
    endif()

    set(_moddir "${CMAKE_SOURCE_DIR}/boost/libs/${_m}")
    if(NOT EXISTS "${_moddir}")
      # module not in tree -> skip
      continue()
    endif()

    # 1) Add top-level <module>/include if present
    if(EXISTS "${_moddir}/include" AND IS_DIRECTORY "${_moddir}/include")
      list(APPEND _collected_includes "${_moddir}/include")
    endif()

    # 2) Recursively discover any include folders under this module (absolute paths).
    # Use LIST_DIRECTORIES true so directories match.
    file(GLOB_RECURSE _found_inc_dirs LIST_DIRECTORIES true "${_moddir}/**/include")
    foreach(_abs_inc IN LISTS _found_inc_dirs)
      if(IS_DIRECTORY "${_abs_inc}")
        list(APPEND _collected_includes "${_abs_inc}")
      endif()
    endforeach()

    # 3) Find header files under the module and add their parent dirs (captures headers without 'include' folders).
    file(GLOB_RECURSE _found_headers LIST_DIRECTORIES false
         "${_moddir}/**/*.hpp" "${_moddir}/**/*.h" "${_moddir}/**/*.ipp" "${_moddir}/**/*.inl")
    foreach(_hdr_abs IN LISTS _found_headers)
      get_filename_component(_hdr_dir "${_hdr_abs}" DIRECTORY)
      if(_hdr_dir AND IS_DIRECTORY "${_hdr_dir}")
        list(APPEND _collected_includes "${_hdr_dir}")
      endif()
    endforeach()

    # 4) Ensure immediate child submodules are included if they contain headers or include/ subdirs.
    #    This covers layouts such as boost/libs/numeric/ublas/*.hpp (no include/) and
    #    boost/libs/numeric/ublas/include/...
    file(GLOB _child_dirs LIST_DIRECTORIES true "${_moddir}/*")
    foreach(_child_abs IN LISTS _child_dirs)
      if(IS_DIRECTORY "${_child_abs}")
        # child include/
        if(EXISTS "${_child_abs}/include" AND IS_DIRECTORY "${_child_abs}/include")
          list(APPEND _collected_includes "${_child_abs}/include")
        else()
          # if any headers exist directly inside the child (or deeper), include the child dir
          file(GLOB_RECURSE _child_hdrs LIST_DIRECTORIES false
               "${_child_abs}/*.hpp" "${_child_abs}/*.h" "${_child_abs}/*.ipp" "${_child_abs}/*.inl")
          if(_child_hdrs)
            list(APPEND _collected_includes "${_child_abs}")
          endif()
        endif()
      endif()
    endforeach()

  endforeach() # modules

  # Always include top-level boost source dir as a fallback
  list(APPEND _collected_includes "${CMAKE_SOURCE_DIR}/boost")

  # Deduplicate and filter only existing absolute directories
  list(REMOVE_DUPLICATES _collected_includes)
  set(_final_includes "")
  foreach(_p IN LISTS _collected_includes)
    if(_p AND IS_DIRECTORY "${_p}")
      list(APPEND _final_includes "${_p}")
    endif()
  endforeach()

  # Export the final list for Wt to use (cache internal to avoid accidental overrides)
  set(BOOST_INCLUDE_DIRS ${_final_includes} CACHE INTERNAL "Collected in-tree Boost include dirs")

  # Debug summary (print count and up to 25 entries)
  list(LENGTH _final_includes _len)
  message(STATUS "WtFindBoost: discovered ${_len} physical include directories under boost/libs (showing up to 25):")
  if(_len GREATER 0)
    if(_len LESS 25)
      set(_max_show ${_len})
    else()
      set(_max_show 25)
    endif()
    foreach(_i RANGE 0 ${_max_show})
      if(_i LESS _len)
        list(GET _final_includes ${_i} _entry)
        message(STATUS "  ${_entry}")
      endif()
    endforeach()
  endif()

  # Set expected flags for Wt
  set(BOOST_WT_FOUND TRUE)
  set(BOOST_WT_MT_FOUND TRUE)
  set(BOOST_WTHTTP_FOUND TRUE)
  set(BOOST_WTHTTP_MT_FOUND TRUE)

  # cleanup temporaries
  unset(_mods)
  unset(_found_inc_dirs)
  unset(_found_headers)
  unset(_child_dirs)
  unset(_child_hdrs)
  unset(_collected_includes)
  unset(_final_includes)

else()
  message(STATUS "WtFindBoost: No in-tree Boost targets found; falling back to find_package(Boost).")
  # fallback to regular find_package behaviour
  find_package(Boost 1.66 REQUIRED COMPONENTS ${Boost_COMPONENTS})
  if(Boost_FOUND)
    if(DEFINED Boost_INCLUDE_DIRS)
      set(BOOST_INCLUDE_DIRS ${Boost_INCLUDE_DIRS})
    endif()
    if(DEFINED Boost_LIBRARY_DIRS)
      set(BOOST_LIB_DIRS ${Boost_LIBRARY_DIRS})
    endif()
  endif()
endif()




#[[ WtFindBoost.txt - patched safe version to support in-tree Boost targets
#
# Sets:
#   BOOST_INCLUDE_DIRS, BOOST_LIB_DIRS, BOOST_WT_LIBRARIES, BOOST_WTHTTP_LIBRARIES, ...
#
# Behaviour:
#  - If Boost::xxx targets already exist (from add_subdirectory(boost)), map them into
#    the BOOST_* variables Wt expects and skip calling find_package()
#  - Otherwise fall back to the original find_package(Boost ...) logic

# default state
SET(BOOST_WT_FOUND FALSE)
SET(BOOST_WT_MT_FOUND FALSE)
SET(BOOST_WTHTTP_FOUND FALSE)
SET(BOOST_WTHTTP_MT_FOUND FALSE)
SET(BOOST_SUPPORT_LIBRARIES "")

IF(NOT DEFINED Boost_ADDITIONAL_VERSIONS)
  SET(Boost_ADDITIONAL_VERSIONS
    "1.66" "1.67" "1.68" "1.69" "1.70" "1.71" "1.72" "1.73" "1.74" "1.75"
    "1.76" "1.77" "1.78" "1.79" "1.80" "1.81" "1.82" "1.83" "1.84" "1.85"
    "1.86" "1.87" "1.88")
ENDIF()

SET(Boost_USE_MULTITHREADED ON)

IF(DEFINED BOOST_PREFIX AND NOT DEFINED BOOST_ROOT)
  SET(BOOST_ROOT ${BOOST_PREFIX})
ENDIF()

IF(WIN32)
  IF(BOOST_DYNAMIC)
    SET(Boost_USE_STATIC_LIBS OFF)
  ELSE()
    SET(Boost_USE_STATIC_LIBS ON)
  ENDIF()
ENDIF()

SET(Boost_DEBUG ON)

SET(Boost_COMPONENTS
    program_options
    thread
)

IF(WT_CPP17_FILESYSTEM_IMPLEMENTATION STREQUAL "boost")
  LIST(APPEND Boost_COMPONENTS filesystem)
ENDIF()

IF(WIN32 AND BOOST_DYNAMIC AND BUILD_TESTS)
  LIST(APPEND Boost_COMPONENTS unit_test_framework)
ENDIF()

#
# --- Detect in-tree Boost targets created by add_subdirectory(boost) ---
#
set(_wt_in_tree_boost FALSE)
set(_wt_boost_includes "")
set(_wt_boost_collected_libs "")

foreach(_comp ${Boost_COMPONENTS})
  if(TARGET Boost::${_comp})
    set(_wt_in_tree_boost TRUE)
    # collect target reference
    list(APPEND _wt_boost_collected_libs Boost::${_comp})
    # if we don't already have include dirs, try to get them from the target
    if(NOT _wt_boost_includes)
      get_target_property(_incdirs Boost::${_comp} INTERFACE_INCLUDE_DIRECTORIES)
      if(_incdirs)
        set(_wt_boost_includes "${_incdirs}")
      endif()
    endif()
  endif()
endforeach()

if(_wt_in_tree_boost)
  # We have in-tree Boost targets. Populate the Wt variables and skip find_package.
  message(STATUS "WtFindBoost: using in-tree Boost targets (Boost::...)")

  set(Boost_FOUND TRUE CACHE INTERNAL "Found in-tree Boost")
  set(BOOST_INCLUDE_DIRS "${_wt_boost_includes}")
  set(BOOST_LIB_DIRS "")

  # Prefer common Wt libs (thread/system/filesystem) if present
  set(_wt_libs "")
  if(TARGET Boost::thread)
    list(APPEND _wt_libs Boost::thread)
    set(BOOST_THREAD_LIB Boost::thread)
  endif()
  if(TARGET Boost::system)
    list(APPEND _wt_libs Boost::system)
    set(BOOST_SYSTEM_LIB Boost::system)
  endif()
  if(TARGET Boost::filesystem)
    list(APPEND _wt_libs Boost::filesystem)
    set(BOOST_FS_LIB Boost::filesystem)
  endif()

  set(BOOST_WT_LIBRARIES ${_wt_libs})
  set(BOOST_WTHTTP_LIBRARIES ${_wt_libs})

  if(TARGET Boost::headers)
    list(APPEND BOOST_WT_LIBRARIES Boost::headers)
    list(APPEND BOOST_WTHTTP_LIBRARIES Boost::headers)
  endif()

  # mark as found
  set(BOOST_WT_FOUND TRUE)
  set(BOOST_WT_MT_FOUND TRUE)
  set(BOOST_WTHTTP_FOUND TRUE)
  set(BOOST_WTHTTP_MT_FOUND TRUE)

  # Clean up temp variables
  unset(_wt_boost_includes)
  unset(_wt_boost_collected_libs)
  unset(_wt_in_tree_boost)
  unset(_wt_libs)
else()
  #
  # --- No in-tree targets — fall back to CMake's find_package(Boost) ---
  #
  function(find_boost_system)
    find_package(Boost 1.66 QUIET COMPONENTS system)
    set(_BOOST_FOUND ${Boost_FOUND} PARENT_SCOPE)
    if(DEFINED Boost_VERSION_MAJOR)
      set(_BOOST_MAJOR_VERSION ${Boost_VERSION_MAJOR} PARENT_SCOPE)
      set(_BOOST_MINOR_VERSION ${Boost_VERSION_MINOR} PARENT_SCOPE)
    else()
      set(_BOOST_MAJOR_VERSION ${Boost_MAJOR_VERSION} PARENT_SCOPE)
      set(_BOOST_MINOR_VERSION ${Boost_MINOR_VERSION} PARENT_SCOPE)
    endif()
  endfunction()

  find_boost_system()

  if(_BOOST_FOUND AND _BOOST_MAJOR_VERSION EQUAL 1 AND _BOOST_MINOR_VERSION LESS 69)
    LIST(APPEND Boost_COMPONENTS system)
  endif()

  # clear temporaries
  set(_BOOST_FOUND)
  set(_BOOST_MAJOR_VERSION)
  set(_BOOST_MINOR_VERSION)

  FIND_PACKAGE(Boost 1.66 REQUIRED COMPONENTS ${Boost_COMPONENTS})

  # If find_package provided IMPORTED targets, map them to Wt vars
  foreach(_target ${Boost_COMPONENTS})
    if(TARGET Boost::${_target})
      string(TOUPPER ${_target} _TARGET_UP)
      set(Boost_${_TARGET_UP}_LIBRARY Boost::${_target})
    endif()
  endforeach()

  if(Boost_FOUND)
    if(Boost_VERSION_MAJOR AND NOT Boost_MAJOR_VERSION)
      set(Boost_MAJOR_VERSION ${Boost_VERSION_MAJOR})
    endif()
    if(Boost_VERSION_MINOR AND NOT Boost_MINOR_VERSION)
      set(Boost_MINOR_VERSION ${Boost_VERSION_MINOR})
    endif()

    # Translate official CMake vars to Wt expected names (safe guards)
    if(DEFINED Boost_INCLUDE_DIRS)
      set(BOOST_INCLUDE_DIRS ${Boost_INCLUDE_DIRS})
    endif()
    if(DEFINED Boost_LIBRARY_DIRS)
      set(BOOST_LIB_DIRS ${Boost_LIBRARY_DIRS})
    endif()
    if(DEFINED Boost_FILESYSTEM_LIBRARY)
      set(BOOST_FS_LIB ${Boost_FILESYSTEM_LIBRARY})
    endif()
    if(DEFINED Boost_PROGRAM_OPTIONS_LIBRARY)
      set(BOOST_PO_LIB ${Boost_PROGRAM_OPTIONS_LIBRARY})
    endif()
    if(DEFINED Boost_SYSTEM_LIBRARY)
      set(BOOST_SYSTEM_LIB ${Boost_SYSTEM_LIBRARY})
    endif()
    if(DEFINED Boost_THREAD_LIBRARY)
      set(BOOST_THREAD_LIB ${Boost_THREAD_LIBRARY})
    endif()

    if(WIN32 AND BOOST_DYNAMIC AND BUILD_TESTS)
      set(BOOST_UT_LIB ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
    else()
      set(BOOST_UT_LIB "")
    endif()

    set(BOOST_WT_MT_FOUND TRUE)
    set(BOOST_WT_FOUND TRUE)
    set(BOOST_WTHTTP_MT_FOUND TRUE)
    set(BOOST_WTHTTP_FOUND TRUE)

    if(MSVC AND Boost_MAJOR_VERSION EQUAL 1 AND Boost_MINOR_VERSION LESS 70)
      set(BOOST_WT_LIBRARIES "")
      set(BOOST_WTHTTP_LIBRARIES "")
      set(BOOST_TEST_LIBRARIES "")
    else()
      set(BOOST_WT_LIBRARIES
        ${Boost_THREAD_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_FILESYSTEM_LIBRARY}
      )
      set(BOOST_WTHTTP_LIBRARIES
        ${Boost_THREAD_LIBRARY}
        ${Boost_PROGRAM_OPTIONS_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_FILESYSTEM_LIBRARY}
      )
      set(BOOST_TEST_LIBRARIES ${BOOST_UT_LIB})
    endif()

    if(TARGET Boost::headers)
      set(BOOST_WT_LIBRARIES ${BOOST_WT_LIBRARIES} Boost::headers)
      set(BOOST_WTHTTP_LIBRARIES ${BOOST_WTHTTP_LIBRARIES} Boost::headers)
    endif()
  endif()
endif()]]


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ END ORIGINAL @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@


# WtFindBoost.txt - in-tree Boost detection with comprehensive per-component include discovery
#
# Behavior:
#  - If Boost is in-tree (Boost:: targets exist), collect include dirs by scanning
#    only the modules listed in Boost_COMPONENTS plus BOOST_INCLUDE_LIBRARIES (if present).
#    For each module:
#      * collect all '**/.../include' directories under the module
#      * for any header found, add the nearest 'include' ancestor (if exists) or the header's dir
#      * add immediate child subfolders that contain header files (covers layouts like numeric/ublas)
#  - Deduplicate and set BOOST_INCLUDE_DIRS
#  - Otherwise fall back to find_package(Boost)
#

# default state
SET(BOOST_WT_FOUND FALSE)
SET(BOOST_WT_MT_FOUND FALSE)
SET(BOOST_WTHTTP_FOUND FALSE)
SET(BOOST_WTHTTP_MT_FOUND FALSE)
SET(BOOST_SUPPORT_LIBRARIES "")

IF(NOT DEFINED Boost_ADDITIONAL_VERSIONS)
  SET(Boost_ADDITIONAL_VERSIONS
    "1.66" "1.67" "1.68" "1.69" "1.70" "1.71" "1.72" "1.73" "1.74" "1.75"
    "1.76" "1.77" "1.78" "1.79" "1.80" "1.81" "1.82" "1.83" "1.84" "1.85"
    "1.86" "1.87" "1.88")
ENDIF()

SET(Boost_USE_MULTITHREADED ON)

IF(DEFINED BOOST_PREFIX AND NOT DEFINED BOOST_ROOT)
  SET(BOOST_ROOT ${BOOST_PREFIX})
ENDIF()

IF(WIN32)
  IF(BOOST_DYNAMIC)
    SET(Boost_USE_STATIC_LIBS OFF)
  ELSE()
    SET(Boost_USE_STATIC_LIBS ON)
  ENDIF()
ENDIF()

SET(Boost_DEBUG ON)

# Components Wt may use (adjust as needed)
SET(Boost_COMPONENTS
    program_options
    thread
    asio
    filesystem
    chrono
    system
    numeric
)

IF(WT_CPP17_FILESYSTEM_IMPLEMENTATION STREQUAL "boost")
  LIST(APPEND Boost_COMPONENTS filesystem)
ENDIF()

IF(WIN32 AND BOOST_DYNAMIC AND BUILD_TESTS)
  LIST(APPEND Boost_COMPONENTS unit_test_framework)
ENDIF()

#
# --- Detect in-tree Boost targets created by add_subdirectory(boost) ---
#
set(_wt_in_tree_boost FALSE)
set(_wt_boost_includes "")
set(_wt_boost_collected_libs "")

foreach(_comp ${Boost_COMPONENTS})
  if(TARGET Boost::${_comp})
    set(_wt_in_tree_boost TRUE)
    list(APPEND _wt_boost_collected_libs Boost::${_comp})
    if(NOT _wt_boost_includes)
      get_target_property(_incdirs Boost::${_comp} INTERFACE_INCLUDE_DIRECTORIES)
      if(_incdirs)
        set(_wt_boost_includes "${_incdirs}")
      endif()
    endif()
  endif()
endforeach()

if(_wt_in_tree_boost)
  message(STATUS "WtFindBoost: using in-tree Boost targets (Boost::...)")
  set(Boost_FOUND TRUE CACHE INTERNAL "Found in-tree Boost")
  set(BOOST_LIB_DIRS "")

  # Map common Boost:: targets into Wt variables if present
  set(_wt_libs "")
  if(TARGET Boost::thread)
    list(APPEND _wt_libs Boost::thread)
    set(BOOST_THREAD_LIB Boost::thread)
  endif()
  if(TARGET Boost::system)
    list(APPEND _wt_libs Boost::system)
    set(BOOST_SYSTEM_LIB Boost::system)
  endif()
  if(TARGET Boost::filesystem)
    list(APPEND _wt_libs Boost::filesystem)
    set(BOOST_FS_LIB Boost::filesystem)
  endif()
  if(TARGET Boost::asio)
    list(APPEND _wt_libs Boost::asio)
    set(BOOST_ASIO_LIB Boost::asio)
  endif()
  if(TARGET Boost::chrono)
    list(APPEND _wt_libs Boost::chrono)
    set(BOOST_CHRONO_LIB Boost::chrono)
  endif()
  if(TARGET Boost::numeric)
    list(APPEND _wt_libs Boost::numeric)
    set(BOOST_NUMERIC_LIB Boost::numeric)
  endif()

  set(BOOST_WT_LIBRARIES ${_wt_libs})
  set(BOOST_WTHTTP_LIBRARIES ${_wt_libs})

  if(TARGET Boost::headers)
    list(APPEND BOOST_WT_LIBRARIES Boost::headers)
    list(APPEND BOOST_WTHTTP_LIBRARIES Boost::headers)
  endif()

  set(BOOST_WT_FOUND TRUE)
  set(BOOST_WT_MT_FOUND TRUE)
  set(BOOST_WTHTTP_FOUND TRUE)
  set(BOOST_WTHTTP_MT_FOUND TRUE)

  #
  # --- Per-component exhaustive include collection ---
  #
  set(_collected_includes "")

  # 1) preserve any INTERFACE include dirs discovered via targets
  if(_wt_boost_includes)
    list(APPEND _collected_includes ${_wt_boost_includes})
  endif()

  # 2) include dirs from Boost::headers umbrella target
  if(TARGET Boost::headers)
    get_target_property(_hdrincs Boost::headers INTERFACE_INCLUDE_DIRECTORIES)
    if(_hdrincs)
      list(APPEND _collected_includes ${_hdrincs})
    endif()
  endif()

  # 3) build the list of modules to scan: union of BOOST_INCLUDE_LIBRARIES (cache) + Boost_COMPONENTS
  set(_mods_to_scan "")
  if(DEFINED BOOST_INCLUDE_LIBRARIES)
    list(APPEND _mods_to_scan ${BOOST_INCLUDE_LIBRARIES})
  endif()
  list(APPEND _mods_to_scan ${Boost_COMPONENTS})
  list(REMOVE_DUPLICATES _mods_to_scan)

  # 4) For each module, scan only that module tree and add:
  #    a) all directories named 'include' under the module (any depth)
  #    b) for any header file found, add the nearest 'include' ancestor (if exists) or the header's dir
  #    c) ensure immediate child subfolders of the module which contain headers are added
  foreach(_mod IN LISTS _mods_to_scan)
    string(STRIP "${_mod}" _mod_trim)
    if(NOT _mod_trim)
      continue()
    endif()

    set(_moddir "${CMAKE_SOURCE_DIR}/boost/libs/${_mod_trim}")
    if(NOT EXISTS "${_moddir}")
      continue()
    endif()

    # 4a) collect directories named 'include' under this module
    file(GLOB_RECURSE _inc_dirs RELATIVE "${_moddir}" "${_moddir}/**/include")
    foreach(_rel IN LISTS _inc_dirs)
      set(_abs "${_moddir}/${_rel}")
      if(EXISTS "${_abs}" AND IS_DIRECTORY "${_abs}")
        list(APPEND _collected_includes "${_abs}")
      endif()
    endforeach()

    # 4b) collect header files and add nearest 'include' ancestor (if any) or header's dir
    file(GLOB_RECURSE _hdrs RELATIVE "${_moddir}"
         "${_moddir}/**/*.hpp" "${_moddir}/**/*.h" "${_moddir}/**/*.ipp" "${_moddir}/**/*.inl")
    foreach(_hdr IN LISTS _hdrs)
      set(_hdr_full "${_moddir}/${_hdr}")
      get_filename_component(_hdr_dir "${_hdr_full}" DIRECTORY)
      if(NOT _hdr_dir)
        continue()
      endif()

      # climb upward from _hdr_dir seeking a directory named 'include'
      set(_search "${_hdr_dir}")
      set(_found_include_dir "")
      while(TRUE)
        get_filename_component(_name "${_search}" NAME)
        if(_name STREQUAL "include")
          set(_found_include_dir "${_search}")
          break()
        endif()
        get_filename_component(_parent "${_search}" DIRECTORY)
        if(_parent STREQUAL _search OR NOT _parent)
          break()
        endif()
        set(_search "${_parent}")
      endwhile()

      if(_found_include_dir)
        list(APPEND _collected_includes "${_found_include_dir}")
      else()
        list(APPEND _collected_includes "${_hdr_dir}")
      endif()
    endforeach()

    # 4c) Ensure immediate child subfolders of the module that contain headers are added.
    # This covers layouts like:
    #   boost/libs/numeric/ublas/*.hpp   (no 'include' dir)
    #   boost/libs/numeric/odeint/include/...
    file(GLOB _child_dirs RELATIVE "${_moddir}" "${_moddir}/*")
    foreach(_child IN LISTS _child_dirs)
      set(_child_abs "${_moddir}/${_child}")
      if(IS_DIRECTORY "${_child_abs}")
        # check if this child dir (or any files inside it) contain headers
        file(GLOB _child_hdrs
             "${_child_abs}/*.hpp" "${_child_abs}/*.h" "${_child_abs}/*.ipp" "${_child_abs}/*.inl")
        if(_child_hdrs)
          list(APPEND _collected_includes "${_child_abs}")
        else()
          # also check nested header files one level deeper (in case of sub-subfolders)
          file(GLOB_RECURSE _child_hdrs2 RELATIVE "${_child_abs}" "${_child_abs}/**/*.hpp" "${_child_abs}/**/*.h")
          if(_child_hdrs2)
            list(APPEND _collected_includes "${_child_abs}")
          endif()
        endif()
      endif()
    endforeach()

  endforeach()

  # 5) final fallback: top-level boost dir
  list(APPEND _collected_includes "${CMAKE_SOURCE_DIR}/boost")

  # 6) deduplicate and set the variable Wt expects
  list(REMOVE_DUPLICATES _collected_includes)
  set(BOOST_INCLUDE_DIRS ${_collected_includes})

  # small debug summary (helps confirm discovery)
  list(LENGTH _collected_includes _ci_len)
  message(STATUS "WtFindBoost: collected ${_ci_len} Boost include directories (first few):")
  foreach(_i RANGE 0 4)
    if(_i LESS _ci_len)
      list(GET _collected_includes ${_i} _t)
      message(STATUS "  ${_t}")
    endif()
  endforeach()

  # cleanup temp variables
  unset(_wt_boost_includes)
  unset(_wt_boost_collected_libs)
  unset(_wt_in_tree_boost)
  unset(_wt_libs)
  unset(_collected_includes)
  unset(_mods_to_scan)
  unset(_inc_dirs)
  unset(_hdrs)
  unset(_child_dirs)
  unset(_child_hdrs)
  unset(_child_hdrs2)

else()
  #
  # --- No in-tree targets — fall back to find_package(Boost) ---
  #
  function(find_boost_system)
    find_package(Boost 1.66 QUIET COMPONENTS system)
    set(_BOOST_FOUND ${Boost_FOUND} PARENT_SCOPE)
    if(DEFINED Boost_VERSION_MAJOR)
      set(_BOOST_MAJOR_VERSION ${Boost_VERSION_MAJOR} PARENT_SCOPE)
      set(_BOOST_MINOR_VERSION ${Boost_VERSION_MINOR} PARENT_SCOPE)
    else()
      set(_BOOST_MAJOR_VERSION ${Boost_MAJOR_VERSION} PARENT_SCOPE)
      set(_BOOST_MINOR_VERSION ${Boost_MINOR_VERSION} PARENT_SCOPE)
    endif()
  endfunction()

  find_boost_system()

  if(_BOOST_FOUND AND _BOOST_MAJOR_VERSION EQUAL 1 AND _BOOST_MINOR_VERSION LESS 69)
    LIST(APPEND Boost_COMPONENTS system)
  endif()

  # clear temporaries
  set(_BOOST_FOUND)
  set(_BOOST_MAJOR_VERSION)
  set(_BOOST_MINOR_VERSION)

  FIND_PACKAGE(Boost 1.66 REQUIRED COMPONENTS ${Boost_COMPONENTS})

  # If find_package provided IMPORTED targets, map them to Wt vars
  foreach(_target ${Boost_COMPONENTS})
    if(TARGET Boost::${_target})
      string(TOUPPER ${_target} _TARGET_UP)
      set(Boost_${_TARGET_UP}_LIBRARY Boost::${_target})
    endif()
  endforeach()

  if(Boost_FOUND)
    if(Boost_VERSION_MAJOR AND NOT Boost_MAJOR_VERSION)
      set(Boost_MAJOR_VERSION ${Boost_VERSION_MAJOR})
    endif()
    if(Boost_VERSION_MINOR AND NOT Boost_MINOR_VERSION)
      set(Boost_MINOR_VERSION ${Boost_VERSION_MINOR})
    endif()

    # Translate official CMake vars to Wt expected names (safe guards)
    if(DEFINED Boost_INCLUDE_DIRS)
      set(BOOST_INCLUDE_DIRS ${Boost_INCLUDE_DIRS})
    endif()
    if(DEFINED Boost_LIBRARY_DIRS)
      set(BOOST_LIB_DIRS ${Boost_LIBRARY_DIRS})
    endif()
    if(DEFINED Boost_FILESYSTEM_LIBRARY)
      set(BOOST_FS_LIB ${Boost_FILESYSTEM_LIBRARY})
    endif()
    if(DEFINED Boost_PROGRAM_OPTIONS_LIBRARY)
      set(BOOST_PO_LIB ${Boost_PROGRAM_OPTIONS_LIBRARY})
    endif()
    if(DEFINED Boost_SYSTEM_LIBRARY)
      set(BOOST_SYSTEM_LIB ${Boost_SYSTEM_LIBRARY})
    endif()
    if(DEFINED Boost_THREAD_LIBRARY)
      set(BOOST_THREAD_LIB ${Boost_THREAD_LIBRARY})
    endif()

    if(WIN32 AND BOOST_DYNAMIC AND BUILD_TESTS)
      set(BOOST_UT_LIB ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
    else()
      set(BOOST_UT_LIB "")
    endif()

    set(BOOST_WT_MT_FOUND TRUE)
    set(BOOST_WT_FOUND TRUE)
    set(BOOST_WTHTTP_MT_FOUND TRUE)
    set(BOOST_WTHTTP_FOUND TRUE)

    if(MSVC AND Boost_MAJOR_VERSION EQUAL 1 AND Boost_MINOR_VERSION LESS 70)
      set(BOOST_WT_LIBRARIES "")
      set(BOOST_WTHTTP_LIBRARIES "")
      set(BOOST_TEST_LIBRARIES "")
    else()
      set(BOOST_WT_LIBRARIES
        ${Boost_THREAD_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_FILESYSTEM_LIBRARY}
      )
      set(BOOST_WTHTTP_LIBRARIES
        ${Boost_THREAD_LIBRARY}
        ${Boost_PROGRAM_OPTIONS_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_FILESYSTEM_LIBRARY}
      )
      set(BOOST_TEST_LIBRARIES ${BOOST_UT_LIB})
    endif()

    if(TARGET Boost::headers)
      set(BOOST_WT_LIBRARIES ${BOOST_WT_LIBRARIES} Boost::headers)
      set(BOOST_WTHTTP_LIBRARIES ${BOOST_WTHTTP_LIBRARIES} Boost::headers)
    endif()
  endif()
endif()

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ SAMPLE THREE @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@


# WtFindBoost.txt - aggressive, absolute-path include discovery for in-tree Boost
#
# Behavior:
#  - If Boost is in-tree (Boost:: targets exist) collect include dirs by:
#      * discovering any 'include' directories under boost/libs/<module> (any depth)
#      * discovering header files and adding their nearest 'include' ancestor (if any)
#        or the header parent dir otherwise
#  - Only scan modules in Boost_COMPONENTS plus any in BOOST_INCLUDE_LIBRARIES (if present)
#  - Deduplicate and set BOOST_INCLUDE_DIRS
#
# Falls back to find_package(Boost) if no in-tree Boost targets are detected.

# default state
SET(BOOST_WT_FOUND FALSE)
SET(BOOST_WT_MT_FOUND FALSE)
SET(BOOST_WTHTTP_FOUND FALSE)
SET(BOOST_WTHTTP_MT_FOUND FALSE)
SET(BOOST_SUPPORT_LIBRARIES "")

IF(NOT DEFINED Boost_ADDITIONAL_VERSIONS)
  SET(Boost_ADDITIONAL_VERSIONS
    "1.66" "1.67" "1.68" "1.69" "1.70" "1.71" "1.72" "1.73" "1.74" "1.75"
    "1.76" "1.77" "1.78" "1.79" "1.80" "1.81" "1.82" "1.83" "1.84" "1.85"
    "1.86" "1.87" "1.88")
ENDIF()

SET(Boost_USE_MULTITHREADED ON)

IF(DEFINED BOOST_PREFIX AND NOT DEFINED BOOST_ROOT)
  SET(BOOST_ROOT ${BOOST_PREFIX})
ENDIF()

IF(WIN32)
  IF(BOOST_DYNAMIC)
    SET(Boost_USE_STATIC_LIBS OFF)
  ELSE()
    SET(Boost_USE_STATIC_LIBS ON)
  ENDIF()
ENDIF()

SET(Boost_DEBUG ON)

# Components Wt may use
SET(Boost_COMPONENTS
    program_options
    thread
    asio
    filesystem
    chrono
    system
    numeric
)

IF(WT_CPP17_FILESYSTEM_IMPLEMENTATION STREQUAL "boost")
  LIST(APPEND Boost_COMPONENTS filesystem)
ENDIF()

IF(WIN32 AND BOOST_DYNAMIC AND BUILD_TESTS)
  LIST(APPEND Boost_COMPONENTS unit_test_framework)
ENDIF()

# --- Detect in-tree Boost targets via add_subdirectory(boost) ---
set(_wt_in_tree_boost FALSE)
set(_wt_boost_includes "")
set(_wt_boost_collected_libs "")

foreach(_comp ${Boost_COMPONENTS})
  if(TARGET Boost::${_comp})
    set(_wt_in_tree_boost TRUE)
    list(APPEND _wt_boost_collected_libs Boost::${_comp})
    if(NOT _wt_boost_includes)
      get_target_property(_incdirs Boost::${_comp} INTERFACE_INCLUDE_DIRECTORIES)
      if(_incdirs)
        set(_wt_boost_includes "${_incdirs}")
      endif()
    endif()
  endif()
endforeach()

if(_wt_in_tree_boost)
  message(STATUS "WtFindBoost: using in-tree Boost targets (Boost::...)")
  set(Boost_FOUND TRUE CACHE INTERNAL "Found in-tree Boost")
  set(BOOST_LIB_DIRS "")

  # Map common Boost:: targets into Wt variables if present
  set(_wt_libs "")
  if(TARGET Boost::thread)
    list(APPEND _wt_libs Boost::thread)
    set(BOOST_THREAD_LIB Boost::thread)
  endif()
  if(TARGET Boost::system)
    list(APPEND _wt_libs Boost::system)
    set(BOOST_SYSTEM_LIB Boost::system)
  endif()
  if(TARGET Boost::filesystem)
    list(APPEND _wt_libs Boost::filesystem)
    set(BOOST_FS_LIB Boost::filesystem)
  endif()
  if(TARGET Boost::asio)
    list(APPEND _wt_libs Boost::asio)
    set(BOOST_ASIO_LIB Boost::asio)
  endif()
  if(TARGET Boost::chrono)
    list(APPEND _wt_libs Boost::chrono)
    set(BOOST_CHRONO_LIB Boost::chrono)
  endif()
  if(TARGET Boost::numeric)
    list(APPEND _wt_libs Boost::numeric)
    set(BOOST_NUMERIC_LIB Boost::numeric)
  endif()

  set(BOOST_WT_LIBRARIES ${_wt_libs})
  set(BOOST_WTHTTP_LIBRARIES ${_wt_libs})

  if(TARGET Boost::headers)
    list(APPEND BOOST_WT_LIBRARIES Boost::headers)
    list(APPEND BOOST_WTHTTP_LIBRARIES Boost::headers)
  endif()

  set(BOOST_WT_FOUND TRUE)
  set(BOOST_WT_MT_FOUND TRUE)
  set(BOOST_WTHTTP_FOUND TRUE)
  set(BOOST_WTHTTP_MT_FOUND TRUE)

  #
  # --- ABSOLUTE-PATH include discovery for selected modules ---
  #
  set(_collected_includes "")
  # preserve any includes exported by targets
  if(_wt_boost_includes)
    list(APPEND _collected_includes ${_wt_boost_includes})
  endif()

  if(TARGET Boost::headers)
    get_target_property(_hdrincs Boost::headers INTERFACE_INCLUDE_DIRECTORIES)
    if(_hdrincs)
      list(APPEND _collected_includes ${_hdrincs})
    endif()
  endif()

  # modules to scan = BOOST_INCLUDE_LIBRARIES (if present) U Boost_COMPONENTS
  set(_mods "")
  if(DEFINED BOOST_INCLUDE_LIBRARIES)
    list(APPEND _mods ${BOOST_INCLUDE_LIBRARIES})
  endif()
  list(APPEND _mods ${Boost_COMPONENTS})
  list(REMOVE_DUPLICATES _mods)

  # For each module, find files under <moddir>/**/include/* (absolute),
  # derive the include dir, and also find headers and climb up from each header.
  foreach(_mod IN LISTS _mods)
    string(STRIP "${_mod}" _m)
    if(NOT _m)
      continue()
    endif()

    set(_moddir "${CMAKE_SOURCE_DIR}/boost/libs/${_m}")
    if(NOT EXISTS "${_moddir}")
      # module not present in source tree -- skip
      continue()
    endif()

    # 1) files under any include dir -> derive the include dir (absolute)
    file(GLOB_RECURSE _files_under_include
      "${_moddir}/**/include/*"
    )
    foreach(_f IN LISTS _files_under_include)
      # _f is absolute path
      get_filename_component(_dir "${_f}" DIRECTORY)
      # climb up until directory name is 'include'
      set(_search "${_dir}")
      while(TRUE)
        get_filename_component(_name "${_search}" NAME)
        if(_name STREQUAL "include")
          list(APPEND _collected_includes "${_search}")
          break()
        endif()
        get_filename_component(_parent "${_search}" DIRECTORY)
        if(_parent STREQUAL _search OR NOT _parent)
          break()
        endif()
        set(_search "${_parent}")
      endwhile()
    endforeach()

    # 2) gather headers anywhere under module and add nearest include ancestor or header dir
    file(GLOB_RECURSE _hdrs
      "${_moddir}/**/*.hpp" "${_moddir}/**/*.h" "${_moddir}/**/*.ipp" "${_moddir}/**/*.inl"
    )
    foreach(_hdr IN LISTS _hdrs)
      # absolute header path
      get_filename_component(_hdrdir "${_hdr}" DIRECTORY)
      if(NOT _hdrdir)
        continue()
      endif()

      # climb up to find 'include' ancestor
      set(_search2 "${_hdrdir}")
      set(_found "")
      while(TRUE)
        get_filename_component(_name2 "${_search2}" NAME)
        if(_name2 STREQUAL "include")
          set(_found "${_search2}")
          break()
        endif()
        get_filename_component(_parent2 "${_search2}" DIRECTORY)
        if(_parent2 STREQUAL _search2 OR NOT _parent2)
          break()
        endif()
        set(_search2 "${_parent2}")
      endwhile()

      if(_found)
        list(APPEND _collected_includes "${_found}")
      else()
        list(APPEND _collected_includes "${_hdrdir}")
      endif()
    endforeach()
  endforeach()

  # last resort: top-level boost dir
  list(APPEND _collected_includes "${CMAKE_SOURCE_DIR}/boost")

  # deduplicate
  list(REMOVE_DUPLICATES _collected_includes)

  # show a compact debug summary for verification
  list(LENGTH _collected_includes _ci_len)
  if(_ci_len GREATER 0)
    message(STATUS "WtFindBoost: discovered ${_ci_len} include directories under boost/libs (will set BOOST_INCLUDE_DIRS).")
  else()
    message(WARNING "WtFindBoost: found no boost include directories under boost/libs — falling back to ${CMAKE_SOURCE_DIR}/boost")
    list(APPEND _collected_includes "${CMAKE_SOURCE_DIR}/boost")
  endif()

  set(BOOST_INCLUDE_DIRS ${_collected_includes})

  # cleanup temporaries
  unset(_wt_boost_includes)
  unset(_wt_boost_collected_libs)
  unset(_wt_in_tree_boost)
  unset(_wt_libs)
  unset(_collected_includes)
  unset(_mods)
  unset(_files_under_include)
  unset(_hdrs)

else()
  #
  # --- fallback to find_package(Boost) ---
  #
  function(find_boost_system)
    find_package(Boost 1.66 QUIET COMPONENTS system)
    set(_BOOST_FOUND ${Boost_FOUND} PARENT_SCOPE)
    if(DEFINED Boost_VERSION_MAJOR)
      set(_BOOST_MAJOR_VERSION ${Boost_VERSION_MAJOR} PARENT_SCOPE)
      set(_BOOST_MINOR_VERSION ${Boost_VERSION_MINOR} PARENT_SCOPE)
    else()
      set(_BOOST_MAJOR_VERSION ${Boost_MAJOR_VERSION} PARENT_SCOPE)
      set(_BOOST_MINOR_VERSION ${Boost_MINOR_VERSION} PARENT_SCOPE)
    endif()
  endfunction()

  find_boost_system()

  if(_BOOST_FOUND AND _BOOST_MAJOR_VERSION EQUAL 1 AND _BOOST_MINOR_VERSION LESS 69)
    LIST(APPEND Boost_COMPONENTS system)
  endif()

  set(_BOOST_FOUND)
  set(_BOOST_MAJOR_VERSION)
  set(_BOOST_MINOR_VERSION)

  FIND_PACKAGE(Boost 1.66 REQUIRED COMPONENTS ${Boost_COMPONENTS})

  foreach(_target ${Boost_COMPONENTS})
    if(TARGET Boost::${_target})
      string(TOUPPER ${_target} _TARGET_UP)
      set(Boost_${_TARGET_UP}_LIBRARY Boost::${_target})
    endif()
  endforeach()

  if(Boost_FOUND)
    if(Boost_VERSION_MAJOR AND NOT Boost_MAJOR_VERSION)
      set(Boost_MAJOR_VERSION ${Boost_VERSION_MAJOR})
    endif()
    if(Boost_VERSION_MINOR AND NOT Boost_MINOR_VERSION)
      set(Boost_MINOR_VERSION ${Boost_VERSION_MINOR})
    endif()

    if(DEFINED Boost_INCLUDE_DIRS)
      set(BOOST_INCLUDE_DIRS ${Boost_INCLUDE_DIRS})
    endif()
    if(DEFINED Boost_LIBRARY_DIRS)
      set(BOOST_LIB_DIRS ${Boost_LIBRARY_DIRS})
    endif()
    if(DEFINED Boost_FILESYSTEM_LIBRARY)
      set(BOOST_FS_LIB ${Boost_FILESYSTEM_LIBRARY})
    endif()
    if(DEFINED Boost_PROGRAM_OPTIONS_LIBRARY)
      set(BOOST_PO_LIB ${Boost_PROGRAM_OPTIONS_LIBRARY})
    endif()
    if(DEFINED Boost_SYSTEM_LIBRARY)
      set(BOOST_SYSTEM_LIB ${Boost_SYSTEM_LIBRARY})
    endif()
    if(DEFINED Boost_THREAD_LIBRARY)
      set(BOOST_THREAD_LIB ${Boost_THREAD_LIBRARY})
    endif()

    if(WIN32 AND BOOST_DYNAMIC AND BUILD_TESTS)
      set(BOOST_UT_LIB ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
    else()
      set(BOOST_UT_LIB "")
    endif()

    set(BOOST_WT_MT_FOUND TRUE)
    set(BOOST_WT_FOUND TRUE)
    set(BOOST_WTHTTP_MT_FOUND TRUE)
    set(BOOST_WTHTTP_FOUND TRUE)

    if(MSVC AND Boost_MAJOR_VERSION EQUAL 1 AND Boost_MINOR_VERSION LESS 70)
      set(BOOST_WT_LIBRARIES "")
      set(BOOST_WTHTTP_LIBRARIES "")
      set(BOOST_TEST_LIBRARIES "")
    else()
      set(BOOST_WT_LIBRARIES
        ${Boost_THREAD_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_FILESYSTEM_LIBRARY}
      )
      set(BOOST_WTHTTP_LIBRARIES
        ${Boost_THREAD_LIBRARY}
        ${Boost_PROGRAM_OPTIONS_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_FILESYSTEM_LIBRARY}
      )
      set(BOOST_TEST_LIBRARIES ${BOOST_UT_LIB})
    endif()

    if(TARGET Boost::headers)
      set(BOOST_WT_LIBRARIES ${BOOST_WT_LIBRARIES} Boost::headers)
      set(BOOST_WTHTTP_LIBRARIES ${BOOST_WTHTTP_LIBRARIES} Boost::headers)
    endif()
  endif()
endif()


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ SAMPLE FOUR @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

# WtFindBoost.txt - detect in-tree Boost and include every 'include' directory
# under boost/libs/* (no per-module hardcoding). Falls back to find_package(Boost).

# default state
SET(BOOST_WT_FOUND FALSE)
SET(BOOST_WT_MT_FOUND FALSE)
SET(BOOST_WTHTTP_FOUND FALSE)
SET(BOOST_WTHTTP_MT_FOUND FALSE)
SET(BOOST_SUPPORT_LIBRARIES "")

IF(NOT DEFINED Boost_ADDITIONAL_VERSIONS)
  SET(Boost_ADDITIONAL_VERSIONS
    "1.66" "1.67" "1.68" "1.69" "1.70" "1.71" "1.72" "1.73" "1.74" "1.75"
    "1.76" "1.77" "1.78" "1.79" "1.80" "1.81" "1.82" "1.83" "1.84" "1.85"
    "1.86" "1.87" "1.88")
ENDIF()

SET(Boost_USE_MULTITHREADED ON)

IF(DEFINED BOOST_PREFIX AND NOT DEFINED BOOST_ROOT)
  SET(BOOST_ROOT ${BOOST_PREFIX})
ENDIF()

IF(WIN32)
  IF(BOOST_DYNAMIC)
    SET(Boost_USE_STATIC_LIBS OFF)
  ELSE()
    SET(Boost_USE_STATIC_LIBS ON)
  ENDIF()
ENDIF()

SET(Boost_DEBUG ON)

# Components Wt may use (you can adjust this list if you want)
SET(Boost_COMPONENTS
    program_options
    thread
    asio
    filesystem
    chrono
    system
    numeric
)

IF(WT_CPP17_FILESYSTEM_IMPLEMENTATION STREQUAL "boost")
  LIST(APPEND Boost_COMPONENTS filesystem)
ENDIF()

IF(WIN32 AND BOOST_DYNAMIC AND BUILD_TESTS)
  LIST(APPEND Boost_COMPONENTS unit_test_framework)
ENDIF()

# --- Detect in-tree Boost targets created by add_subdirectory(boost) ---
set(_wt_in_tree_boost FALSE)
set(_wt_boost_includes "")
set(_wt_boost_collected_libs "")

foreach(_comp ${Boost_COMPONENTS})
  if(TARGET Boost::${_comp})
    set(_wt_in_tree_boost TRUE)
    list(APPEND _wt_boost_collected_libs Boost::${_comp})
    if(NOT _wt_boost_includes)
      get_target_property(_incdirs Boost::${_comp} INTERFACE_INCLUDE_DIRECTORIES)
      if(_incdirs)
        set(_wt_boost_includes "${_incdirs}")
      endif()
    endif()
  endif()
endforeach()

if(_wt_in_tree_boost)
  message(STATUS "WtFindBoost: using in-tree Boost targets (Boost::...)")

  set(Boost_FOUND TRUE CACHE INTERNAL "Found in-tree Boost")
  set(BOOST_LIB_DIRS "")

  # Map common Boost:: targets into Wt variables if present
  set(_wt_libs "")
  if(TARGET Boost::thread)
    list(APPEND _wt_libs Boost::thread)
    set(BOOST_THREAD_LIB Boost::thread)
  endif()
  if(TARGET Boost::system)
    list(APPEND _wt_libs Boost::system)
    set(BOOST_SYSTEM_LIB Boost::system)
  endif()
  if(TARGET Boost::filesystem)
    list(APPEND _wt_libs Boost::filesystem)
    set(BOOST_FS_LIB Boost::filesystem)
  endif()
  if(TARGET Boost::asio)
    list(APPEND _wt_libs Boost::asio)
    set(BOOST_ASIO_LIB Boost::asio)
  endif()
  if(TARGET Boost::chrono)
    list(APPEND _wt_libs Boost::chrono)
    set(BOOST_CHRONO_LIB Boost::chrono)
  endif()
  if(TARGET Boost::numeric)
    list(APPEND _wt_libs Boost::numeric)
    set(BOOST_NUMERIC_LIB Boost::numeric)
  endif()

  set(BOOST_WT_LIBRARIES ${_wt_libs})
  set(BOOST_WTHTTP_LIBRARIES ${_wt_libs})

  if(TARGET Boost::headers)
    list(APPEND BOOST_WT_LIBRARIES Boost::headers)
    list(APPEND BOOST_WTHTTP_LIBRARIES Boost::headers)
  endif()

  set(BOOST_WT_FOUND TRUE)
  set(BOOST_WT_MT_FOUND TRUE)
  set(BOOST_WTHTTP_FOUND TRUE)
  set(BOOST_WTHTTP_MT_FOUND TRUE)

  #
  # --- CORE: Collect every directory literally named "include" under boost/libs/*
  #
  set(_collected_includes "")

  # 1) keep any INTERFACE include dirs discovered via targets
  if(_wt_boost_includes)
    list(APPEND _collected_includes ${_wt_boost_includes})
  endif()

  # 2) also add headers umbrella include dirs if present
  if(TARGET Boost::headers)
    get_target_property(_hdrincs Boost::headers INTERFACE_INCLUDE_DIRECTORIES)
    if(_hdrincs)
      list(APPEND _collected_includes ${_hdrincs})
    endif()
  endif()

  # 3) find every directory named 'include' under boost/libs and add it
  if(EXISTS "${CMAKE_SOURCE_DIR}/boost/libs")
    # Recursively find directories named 'include' under boost/libs
    file(GLOB_RECURSE _found_include_dirs
         RELATIVE "${CMAKE_SOURCE_DIR}/boost"
         "${CMAKE_SOURCE_DIR}/boost/libs/*/**/include")
    foreach(_rel_dir IN LISTS _found_include_dirs)
      # build absolute path and append
      set(_abs "${CMAKE_SOURCE_DIR}/${_rel_dir}")
      if(EXISTS "${_abs}" AND IS_DIRECTORY "${_abs}")
        list(APPEND _collected_includes "${_abs}")
      endif()
    endforeach()
  endif()

  # 4) final fallback: add top-level boost source dir
  list(APPEND _collected_includes "${CMAKE_SOURCE_DIR}/boost")

  # 5) deduplicate and export
  list(REMOVE_DUPLICATES _collected_includes)
  set(BOOST_INCLUDE_DIRS ${_collected_includes})

  # cleanup temporaries
  unset(_wt_boost_includes)
  unset(_wt_boost_collected_libs)
  unset(_wt_in_tree_boost)
  unset(_wt_libs)
  unset(_collected_includes)
  unset(_found_include_dirs)

else()
  #
  # --- No in-tree targets — fall back to find_package(Boost) ---
  #
  function(find_boost_system)
    find_package(Boost 1.66 QUIET COMPONENTS system)
    set(_BOOST_FOUND ${Boost_FOUND} PARENT_SCOPE)
    if(DEFINED Boost_VERSION_MAJOR)
      set(_BOOST_MAJOR_VERSION ${Boost_VERSION_MAJOR} PARENT_SCOPE)
      set(_BOOST_MINOR_VERSION ${Boost_VERSION_MINOR} PARENT_SCOPE)
    else()
      set(_BOOST_MAJOR_VERSION ${Boost_MAJOR_VERSION} PARENT_SCOPE)
      set(_BOOST_MINOR_VERSION ${Boost_MINOR_VERSION} PARENT_SCOPE)
    endif()
  endfunction()

  find_boost_system()

  if(_BOOST_FOUND AND _BOOST_MAJOR_VERSION EQUAL 1 AND _BOOST_MINOR_VERSION LESS 69)
    LIST(APPEND Boost_COMPONENTS system)
  endif()

  set(_BOOST_FOUND)
  set(_BOOST_MAJOR_VERSION)
  set(_BOOST_MINOR_VERSION)

  FIND_PACKAGE(Boost 1.66 REQUIRED COMPONENTS ${Boost_COMPONENTS})

  # Map imported targets to Wt variables if present
  foreach(_target ${Boost_COMPONENTS})
    if(TARGET Boost::${_target})
      string(TOUPPER ${_target} _TARGET_UP)
      set(Boost_${_TARGET_UP}_LIBRARY Boost::${_target})
    endif()
  endforeach()

  if(Boost_FOUND)
    if(Boost_VERSION_MAJOR AND NOT Boost_MAJOR_VERSION)
      set(Boost_MAJOR_VERSION ${Boost_VERSION_MAJOR})
    endif()
    if(Boost_VERSION_MINOR AND NOT Boost_MINOR_VERSION)
      set(Boost_MINOR_VERSION ${Boost_VERSION_MINOR})
    endif()

    if(DEFINED Boost_INCLUDE_DIRS)
      set(BOOST_INCLUDE_DIRS ${Boost_INCLUDE_DIRS})
    endif()
    if(DEFINED Boost_LIBRARY_DIRS)
      set(BOOST_LIB_DIRS ${Boost_LIBRARY_DIRS})
    endif()
    if(DEFINED Boost_FILESYSTEM_LIBRARY)
      set(BOOST_FS_LIB ${Boost_FILESYSTEM_LIBRARY})
    endif()
    if(DEFINED Boost_PROGRAM_OPTIONS_LIBRARY)
      set(BOOST_PO_LIB ${Boost_PROGRAM_OPTIONS_LIBRARY})
    endif()
    if(DEFINED Boost_SYSTEM_LIBRARY)
      set(BOOST_SYSTEM_LIB ${Boost_SYSTEM_LIBRARY})
    endif()
    if(DEFINED Boost_THREAD_LIBRARY)
      set(BOOST_THREAD_LIB ${Boost_THREAD_LIBRARY})
    endif()

    if(WIN32 AND BOOST_DYNAMIC AND BUILD_TESTS)
      set(BOOST_UT_LIB ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
    else()
      set(BOOST_UT_LIB "")
    endif()

    set(BOOST_WT_MT_FOUND TRUE)
    set(BOOST_WT_FOUND TRUE)
    set(BOOST_WTHTTP_MT_FOUND TRUE)
    set(BOOST_WTHTTP_FOUND TRUE)

    if(MSVC AND Boost_MAJOR_VERSION EQUAL 1 AND Boost_MINOR_VERSION LESS 70)
      set(BOOST_WT_LIBRARIES "")
      set(BOOST_WTHTTP_LIBRARIES "")
      set(BOOST_TEST_LIBRARIES "")
    else()
      set(BOOST_WT_LIBRARIES
        ${Boost_THREAD_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_FILESYSTEM_LIBRARY}
      )
      set(BOOST_WTHTTP_LIBRARIES
        ${Boost_THREAD_LIBRARY}
        ${Boost_PROGRAM_OPTIONS_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_FILESYSTEM_LIBRARY}
      )
      set(BOOST_TEST_LIBRARIES ${BOOST_UT_LIB})
    endif()

    if(TARGET Boost::headers)
      set(BOOST_WT_LIBRARIES ${BOOST_WT_LIBRARIES} Boost::headers)
      set(BOOST_WTHTTP_LIBRARIES ${BOOST_WTHTTP_LIBRARIES} Boost::headers)
    endif()
  endif()
endif()


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ SAMPLE FIVE @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@


# WtFindBoost.txt - patched: in-tree Boost detection + filesystem-first include discovery
#
# Behavior:
#  - If Boost::xxx targets already exist (from add_subdirectory(boost)), map them into
#    the BOOST_* variables Wt expects and skip calling find_package()
#  - Additionally, perform a filesystem-first scan for Boost include dirs for modules
#    listed in Boost_COMPONENTS (and BOOST_INCLUDE_LIBRARIES if present).
#  - Ignores generator expressions from target properties and only appends absolute,
#    existing directories to BOOST_INCLUDE_DIRS.
#  - Otherwise fall back to the original find_package(Boost ...) logic.
#
# Based on original WtFindBoost.txt (user-provided). See embedded diagnostics when needed.
#

# default state
set(BOOST_WT_FOUND FALSE)
set(BOOST_WT_MT_FOUND FALSE)
set(BOOST_WTHTTP_FOUND FALSE)
set(BOOST_WTHTTP_MT_FOUND FALSE)
set(BOOST_SUPPORT_LIBRARIES "")

if(NOT DEFINED Boost_ADDITIONAL_VERSIONS)
  set(Boost_ADDITIONAL_VERSIONS
    "1.66" "1.67" "1.68" "1.69" "1.70" "1.71" "1.72" "1.73" "1.74" "1.75"
    "1.76" "1.77" "1.78" "1.79" "1.80" "1.81" "1.82" "1.83" "1.84" "1.85"
    "1.86" "1.87" "1.88")
endif()

set(Boost_USE_MULTITHREADED ON)

if(DEFINED BOOST_PREFIX AND NOT DEFINED BOOST_ROOT)
  set(BOOST_ROOT ${BOOST_PREFIX})
endif()

if(WIN32)
  if(BOOST_DYNAMIC)
    set(Boost_USE_STATIC_LIBS OFF)
  else()
    set(Boost_USE_STATIC_LIBS ON)
  endif()
endif()

set(Boost_DEBUG ON)

# Default components Wt needs (adjust as required)
set(Boost_COMPONENTS
  program_options
  thread
)

if(WT_CPP17_FILESYSTEM_IMPLEMENTATION STREQUAL "boost")
  list(APPEND Boost_COMPONENTS filesystem)
endif()

if(WIN32 AND BOOST_DYNAMIC AND BUILD_TESTS)
  list(APPEND Boost_COMPONENTS unit_test_framework)
endif()

#
# --------- NEW: Detect an in-tree Boost (targets created by add_subdirectory) ---------
#
set(_wt_in_tree_boost FALSE)
set(_wt_boost_interface_includes "")   # keep generator-expr includes for diagnostics
set(_wt_boost_collected_libs "")

foreach(_comp IN LISTS Boost_COMPONENTS)
  if(TARGET Boost::${_comp})
    set(_wt_in_tree_boost TRUE)
    list(APPEND _wt_boost_collected_libs Boost::${_comp})
    # collect INTERFACE_INCLUDE_DIRECTORIES but do not rely on generator expressions
    get_target_property(_incdirs Boost::${_comp} INTERFACE_INCLUDE_DIRECTORIES)
    if(_incdirs)
      list(APPEND _wt_boost_interface_includes ${_incdirs})
    endif()
  endif()
endforeach()

if(_wt_in_tree_boost)
  message(STATUS "WtFindBoost: Boost:: targets detected (in-tree Boost). Using filesystem-first include discovery.")
  set(Boost_FOUND TRUE CACHE INTERNAL "Found in-tree Boost")
  set(BOOST_LIB_DIRS "")

  #
  # --------- Filesystem-first include discovery (component-limited) ----------
  #
  # Goal: for each component in Boost_COMPONENTS and any in BOOST_INCLUDE_LIBRARIES,
  # collect absolute include directories that actually exist on disk:
  #   - <module>/include
  #   - <module>/*/include
  #   - <module>/<child>   (if child contains headers directly)
  #   - header-parent dirs for header files found under module
  #
  set(_collected_includes_raw "")

  # Modules to scan: union of Boost_COMPONENTS and BOOST_INCLUDE_LIBRARIES
  set(_mods "")
  if(DEFINED BOOST_INCLUDE_LIBRARIES)
    list(APPEND _mods ${BOOST_INCLUDE_LIBRARIES})
  endif()
  list(APPEND _mods ${Boost_COMPONENTS})
  list(REMOVE_DUPLICATES _mods)

  foreach(_mod IN LISTS _mods)
    string(STRIP "${_mod}" _mod_trim)
    if(NOT _mod_trim)
      continue()
    endif()

    set(_moddir "${CMAKE_SOURCE_DIR}/boost/libs/${_mod_trim}")
    # If moddir missing, also try current source dir path (some projects configure from subdir)
    if(NOT EXISTS "${_moddir}")
      set(_moddir_alt "${CMAKE_CURRENT_SOURCE_DIR}/boost/libs/${_mod_trim}")
      if(EXISTS "${_moddir_alt}")
        set(_moddir "${_moddir_alt}")
      endif()
    endif()

    if(NOT EXISTS "${_moddir}")
      # module not present in source tree - skip
      message(STATUS "WtFindBoost: module not present, skipping: ${_mod_trim}")
      continue()
    endif()

    # (1) Add module-level include folder if present
    if(EXISTS "${_moddir}/include" AND IS_DIRECTORY "${_moddir}/include")
      list(APPEND _collected_includes_raw "${_moddir}/include")
    endif()

    # (2) Recursively collect any include directories under this module (absolute)
    file(GLOB_RECURSE _found_inc_dirs LIST_DIRECTORIES true "${_moddir}/**/include")
    foreach(_incdir IN LISTS _found_inc_dirs)
      if(IS_DIRECTORY "${_incdir}")
        list(APPEND _collected_includes_raw "${_incdir}")
      endif()
    endforeach()

    # (3) Collect header-parent directories (captures headers in non-include layouts)
    file(GLOB_RECURSE _found_headers LIST_DIRECTORIES false
         "${_moddir}/**/*.hpp" "${_moddir}/**/*.h" "${_moddir}/**/*.ipp" "${_moddir}/**/*.inl")
    foreach(_hdr IN LISTS _found_headers)
      get_filename_component(_hdr_dir "${_hdr}" DIRECTORY)
      if(_hdr_dir AND IS_DIRECTORY "${_hdr_dir}")
        list(APPEND _collected_includes_raw "${_hdr_dir}")
      endif()
    endforeach()

    # (4) Ensure immediate child subfolders (e.g. numeric/ublas) are included if they contain headers
    file(GLOB _child_dirs LIST_DIRECTORIES true "${_moddir}/*")
    foreach(_child_abs IN LISTS _child_dirs)
      if(IS_DIRECTORY "${_child_abs}")
        if(EXISTS "${_child_abs}/include" AND IS_DIRECTORY "${_child_abs}/include")
          list(APPEND _collected_includes_raw "${_child_abs}/include")
        else()
          file(GLOB_RECURSE _child_hdrs LIST_DIRECTORIES false
               "${_child_abs}/*.hpp" "${_child_abs}/*.h" "${_child_abs}/*.ipp" "${_child_abs}/*.inl")
          if(_child_hdrs)
            list(APPEND _collected_includes_raw "${_child_abs}")
          endif()
        endif()
      endif()
    endforeach()

  endforeach() # _mods

  # (5) Always append top-level boost source dir as fallback
  list(APPEND _collected_includes_raw "${CMAKE_SOURCE_DIR}/boost")

  # (6) Deduplicate and keep only existing absolute directories (ignore generator expressions)
  list(REMOVE_DUPLICATES _collected_includes_raw)
  set(_final_includes "")
  foreach(_p IN LISTS _collected_includes_raw)
    # Normalize and skip if looks like generator-expression or not a path
    if("${_p}" MATCHES "\\$<.*:.*>")
      continue()
    endif()
    if(_p AND IS_DIRECTORY "${_p}")
      list(APPEND _final_includes "${_p}")
    endif()
  endforeach()

  # Export as BOOST_INCLUDE_DIRS (CACHE INTERNAL to avoid accidental override)
  set(BOOST_INCLUDE_DIRS ${_final_includes} CACHE INTERNAL "Collected in-tree Boost include dirs")

  # Debug summary: report what we discovered (helpful for verification)
  list(LENGTH _final_includes _len)
  message(STATUS "WtFindBoost: discovered ${_len} physical include directories under boost/libs (showing up to 50):")
  if(_len GREATER 0)
    if(_len LESS 50)
      set(_max_show ${_len})
    else()
      set(_max_show 50)
    endif()
    foreach(_i RANGE 0 ${_max_show})
      if(_i LESS _len)
        list(GET _final_includes ${_i} _show)
        message(STATUS "  ${_show}")
      endif()
    endforeach()
  else()
    message(STATUS "  (no physical include directories discovered via filesystem scan)")
    # If nothing found, as a diagnostic step, also output interface include props (generator-expr) if any
    if(_wt_boost_interface_includes)
      message(STATUS "  Note: Boost targets expose INTERFACE include properties (generator-expr); showing them for debugging:")
      foreach(_gi IN LISTS _wt_boost_interface_includes)
        message(STATUS "    ${_gi}")
      endforeach()
    endif()
  endif()

  # Done: map discovered targets to BOOST_* variables (set as a result of discovery)
  set(_wt_libs "")
  foreach(_lib IN ITEMS program_options thread system filesystem asio chrono numeric)
    if(TARGET Boost::${_lib})
      list(APPEND _wt_libs Boost::${_lib})
      string(TOUPPER ${_lib} _ULIB)
      set(BOOST_${_ULIB}_LIB Boost::${_lib})
    endif()
  endforeach()

  set(BOOST_WT_LIBRARIES ${_wt_libs})
  set(BOOST_WTHTTP_LIBRARIES ${_wt_libs})

  if(TARGET Boost::headers)
    list(APPEND BOOST_WT_LIBRARIES Boost::headers)
    list(APPEND BOOST_WTHTTP_LIBRARIES Boost::headers)
  endif()

  # Final presence flags (set after detection and mapping)
  set(BOOST_WT_FOUND TRUE)
  set(BOOST_WT_MT_FOUND TRUE)
  set(BOOST_WTHTTP_FOUND TRUE)
  set(BOOST_WTHTTP_MT_FOUND TRUE)

  # Cleanup temporaries
  unset(_collected_includes_raw)
  unset(_found_inc_dirs)
  unset(_found_headers)
  unset(_child_dirs)
  unset(_child_hdrs)
  unset(_final_includes)
  unset(_mods)
  unset(_wt_boost_interface_includes)
  unset(_wt_boost_collected_libs)
  unset(_wt_in_tree_boost)
  unset(_wt_libs)

#
# --------- END in-tree detection and filesystem discovery -------------------
#
else()

  #
  # --- No in-tree targets — fall back to CMake's find_package(Boost) ---
  #
  function(find_boost_system)
    find_package(Boost 1.66 QUIET
      COMPONENTS
        system
    )

    set(_BOOST_FOUND ${Boost_FOUND} PARENT_SCOPE)
    if(Boost_VERSION_MAJOR)
      set(_BOOST_MAJOR_VERSION ${Boost_VERSION_MAJOR} PARENT_SCOPE)
      set(_BOOST_MINOR_VERSION ${Boost_VERSION_MINOR} PARENT_SCOPE)
    else()
      set(_BOOST_MAJOR_VERSION ${Boost_MAJOR_VERSION} PARENT_SCOPE)
      set(_BOOST_MINOR_VERSION ${Boost_MINOR_VERSION} PARENT_SCOPE)
    endif()
  endfunction()

  find_boost_system()

  if(_BOOST_FOUND AND _BOOST_MAJOR_VERSION EQUAL 1 AND _BOOST_MINOR_VERSION LESS 69)
    list(APPEND Boost_COMPONENTS system)
  endif()

  set(_BOOST_FOUND)
  set(_BOOST_MAJOR_VERSION)
  set(_BOOST_MINOR_VERSION)

  find_package(Boost 1.66
    COMPONENTS
      ${Boost_COMPONENTS}
    REQUIRED
  )

  # Use IMPORTED targets if defined (map to Wt variables)
  foreach(_target IN LISTS Boost_COMPONENTS)
    if(TARGET Boost::${_target})
      string(TOUPPER ${_target} _TARGET)
      set(Boost_${_TARGET}_LIBRARY Boost::${_target})
    endif()
  endforeach()
  unset(_target)
  unset(_TARGET)

  if(Boost_FOUND)
    if(Boost_VERSION_MAJOR AND NOT Boost_MAJOR_VERSION)
      set(Boost_MAJOR_VERSION ${Boost_VERSION_MAJOR})
    endif()
    if(Boost_VERSION_MINOR AND NOT Boost_MINOR_VERSION)
      set(Boost_MINOR_VERSION ${Boost_VERSION_MINOR})
    endif()

    # Translate variables from official cmake names to internally used names
    if(NOT DEFINED BOOST_INCLUDE_DIRS AND DEFINED Boost_INCLUDE_DIRS)
      set(BOOST_INCLUDE_DIRS ${Boost_INCLUDE_DIRS})
    endif()
    if(NOT DEFINED BOOST_LIB_DIRS AND DEFINED Boost_LIBRARY_DIRS)
      set(BOOST_LIB_DIRS ${Boost_LIBRARY_DIRS})
    endif()

    # If find_package actually set variables for individual libs, map them:
    if(DEFINED Boost_FILESYSTEM_LIBRARY)
      set(BOOST_FS_LIB ${Boost_FILESYSTEM_LIBRARY})
    endif()
    if(DEFINED Boost_PROGRAM_OPTIONS_LIBRARY)
      set(BOOST_PO_LIB ${Boost_PROGRAM_OPTIONS_LIBRARY})
    endif()
    if(DEFINED Boost_SYSTEM_LIBRARY)
      set(BOOST_SYSTEM_LIB ${Boost_SYSTEM_LIBRARY})
    endif()
    if(DEFINED Boost_THREAD_LIBRARY)
      set(BOOST_THREAD_LIB ${Boost_THREAD_LIBRARY})
    endif()
    if(WIN32 AND BOOST_DYNAMIC AND BUILD_TESTS)
      set(BOOST_UT_LIB ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
    else()
      if(NOT DEFINED BOOST_UT_LIB)
        set(BOOST_UT_LIB "")
      endif()
    endif()

    set(BOOST_WT_MT_FOUND TRUE)
    set(BOOST_WT_FOUND TRUE)
    set(BOOST_WTHTTP_MT_FOUND TRUE)
    set(BOOST_WTHTTP_FOUND TRUE)

    if(MSVC AND Boost_MAJOR_VERSION EQUAL 1 AND Boost_MINOR_VERSION LESS 70)
      # use autolink
      set(BOOST_WT_LIBRARIES "")
      set(BOOST_WTHTTP_LIBRARIES "")
      set(BOOST_TEST_LIBRARIES "")
    else()
      # prefer explicit variables if available, otherwise rely on the Boost::<lib> targets we set above
      set(BOOST_WT_LIBRARIES
        ${Boost_THREAD_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_FILESYSTEM_LIBRARY})
      set(BOOST_WTHTTP_LIBRARIES
        ${Boost_THREAD_LIBRARY}
        ${Boost_PROGRAM_OPTIONS_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_FILESYSTEM_LIBRARY})
      set(BOOST_TEST_LIBRARIES ${BOOST_UT_LIB})
    endif()

    if(TARGET Boost::headers)
      set(BOOST_WT_LIBRARIES ${BOOST_WT_LIBRARIES} Boost::headers)
      set(BOOST_WTHTTP_LIBRARIES ${BOOST_WTHTTP_LIBRARIES} Boost::headers)
    endif()
  endif()
endif()


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ SAMPLE SIX @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@


# WtFindBoost.txt - patched safe version to support in-tree Boost targets
#
# Sets:
#   BOOST_INCLUDE_DIRS, BOOST_LIB_DIRS, BOOST_WT_LIBRARIES, BOOST_WTHTTP_LIBRARIES, ...
#
# Behaviour:
#  - If Boost::xxx targets already exist (from add_subdirectory(boost)), map them into
#    the BOOST_* variables Wt expects and skip calling find_package()
#  - Otherwise fall back to the original find_package(Boost ...) logic

# default state
SET(BOOST_WT_FOUND FALSE)
SET(BOOST_WT_MT_FOUND FALSE)
SET(BOOST_WTHTTP_FOUND FALSE)
SET(BOOST_WTHTTP_MT_FOUND FALSE)
SET(BOOST_SUPPORT_LIBRARIES "")

IF(NOT DEFINED Boost_ADDITIONAL_VERSIONS)
  SET(Boost_ADDITIONAL_VERSIONS
    "1.66" "1.67" "1.68" "1.69" "1.70" "1.71" "1.72" "1.73" "1.74" "1.75"
    "1.76" "1.77" "1.78" "1.79" "1.80" "1.81" "1.82" "1.83" "1.84" "1.85"
    "1.86" "1.87" "1.88")
ENDIF()

SET(Boost_USE_MULTITHREADED ON)

IF(DEFINED BOOST_PREFIX AND NOT DEFINED BOOST_ROOT)
  SET(BOOST_ROOT ${BOOST_PREFIX})
ENDIF()

IF(WIN32)
  IF(BOOST_DYNAMIC)
    SET(Boost_USE_STATIC_LIBS OFF)
  ELSE()
    SET(Boost_USE_STATIC_LIBS ON)
  ENDIF()
ENDIF()

SET(Boost_DEBUG ON)

# Explicit components Wt may use — added asio and chrono explicitly.
SET(Boost_COMPONENTS
    program_options
    thread
    asio
    filesystem
    chrono
    system
)

IF(WT_CPP17_FILESYSTEM_IMPLEMENTATION STREQUAL "boost")
  # filesystem may already be listed; this keeps the old conditional behavior as well
  LIST(APPEND Boost_COMPONENTS filesystem)
ENDIF()

IF(WIN32 AND BOOST_DYNAMIC AND BUILD_TESTS)
  LIST(APPEND Boost_COMPONENTS unit_test_framework)
ENDIF()

#
# --- Detect in-tree Boost targets created by add_subdirectory(boost) ---
#
set(_wt_in_tree_boost FALSE)
set(_wt_boost_includes "")
set(_wt_boost_collected_libs "")

foreach(_comp ${Boost_COMPONENTS})
  if(TARGET Boost::${_comp})
    set(_wt_in_tree_boost TRUE)
    # collect target reference
    list(APPEND _wt_boost_collected_libs Boost::${_comp})
    # if we don't already have include dirs, try to get them from the target
    if(NOT _wt_boost_includes)
      get_target_property(_incdirs Boost::${_comp} INTERFACE_INCLUDE_DIRECTORIES)
      if(_incdirs)
        set(_wt_boost_includes "${_incdirs}")
      endif()
    endif()
  endif()
endforeach()

if(_wt_in_tree_boost)
  # We have in-tree Boost targets. Populate the Wt variables and skip find_package.
  message(STATUS "WtFindBoost: using in-tree Boost targets (Boost::...)")

  set(Boost_FOUND TRUE CACHE INTERNAL "Found in-tree Boost")
  set(BOOST_LIB_DIRS "")

  # Prefer common Wt libs (thread/system/filesystem) if present
  set(_wt_libs "")
  if(TARGET Boost::thread)
    list(APPEND _wt_libs Boost::thread)
    set(BOOST_THREAD_LIB Boost::thread)
  endif()
  if(TARGET Boost::system)
    list(APPEND _wt_libs Boost::system)
    set(BOOST_SYSTEM_LIB Boost::system)
  endif()
  if(TARGET Boost::filesystem)
    list(APPEND _wt_libs Boost::filesystem)
    set(BOOST_FS_LIB Boost::filesystem)
  endif()

  # Add asio if present
  if(TARGET Boost::asio)
    list(APPEND _wt_libs Boost::asio)
    set(BOOST_ASIO_LIB Boost::asio)
  endif()

  set(BOOST_WT_LIBRARIES ${_wt_libs})
  set(BOOST_WTHTTP_LIBRARIES ${_wt_libs})

  if(TARGET Boost::headers)
    list(APPEND BOOST_WT_LIBRARIES Boost::headers)
    list(APPEND BOOST_WTHTTP_LIBRARIES Boost::headers)
  endif()

  # mark as found
  set(BOOST_WT_FOUND TRUE)
  set(BOOST_WT_MT_FOUND TRUE)
  set(BOOST_WTHTTP_FOUND TRUE)
  set(BOOST_WTHTTP_MT_FOUND TRUE)

  #
  # --- Robust collection of Boost include directories (uses BOOST_INCLUDE_LIBRARIES if available) ---
  #
  set(_collected_includes "")

  # a) keep any include dirs we obtained earlier
  if(_wt_boost_includes)
    list(APPEND _collected_includes ${_wt_boost_includes})
  endif()

  # b) include dirs from Boost::headers umbrella target
  if(TARGET Boost::headers)
    get_target_property(_hdrincs Boost::headers INTERFACE_INCLUDE_DIRECTORIES)
    if(_hdrincs)
      list(APPEND _collected_includes ${_hdrincs})
    endif()
  endif()

  # c) include INTERFACE dirs for each collected Boost::<component> target (if present)
  foreach(_bt ${_wt_boost_collected_libs})
    if(TARGET ${_bt})
      get_target_property(_incs ${_bt} INTERFACE_INCLUDE_DIRECTORIES)
      if(_incs)
        list(APPEND _collected_includes ${_incs})
      endif()
    endif()
  endforeach()

  # d) If the Boost superproject exported a cache variable listing header-only libs, use it.
  #    This matches BOOST_INCLUDE_LIBRARIES (explicit list of libs).
  if(DEFINED BOOST_INCLUDE_LIBRARIES)
    foreach(_lib IN LISTS BOOST_INCLUDE_LIBRARIES)
      string(STRIP "${_lib}" _lib_trim)
      if(_lib_trim)
        set(_candidate "${CMAKE_SOURCE_DIR}/boost/libs/${_lib_trim}/include")
        if(EXISTS "${_candidate}")
          list(APPEND _collected_includes "${_candidate}")
        else()
          # Try a common variant if a nested 'boost' subdir exists
          if(EXISTS "${_candidate}/boost")
            list(APPEND _collected_includes "${_candidate}")
          endif()
        endif()
      endif()
    endforeach()
  else()
    # e) Fallback: scan all libs/*/include existing directories (previous approach)
    if(EXISTS "${CMAKE_SOURCE_DIR}/boost/libs")
      file(GLOB _boost_lib_dirs RELATIVE "${CMAKE_SOURCE_DIR}/boost/libs" "${CMAKE_SOURCE_DIR}/boost/libs/*")
      foreach(_ldir ${_boost_lib_dirs})
        set(_candidate "${CMAKE_SOURCE_DIR}/boost/libs/${_ldir}/include")
        if(EXISTS "${_candidate}")
          list(APPEND _collected_includes "${_candidate}")
        endif()
      endforeach()
    endif()
  endif()

  # f) final fallback: top-level boost dir
  list(APPEND _collected_includes "${CMAKE_SOURCE_DIR}/boost")

  # remove duplicates and set the var Wt expects
  list(REMOVE_DUPLICATES _collected_includes)
  set(BOOST_INCLUDE_DIRS ${_collected_includes})
  # ----------------------------------------------------------------

  # Clean up temp variables
  unset(_wt_boost_includes)
  unset(_wt_boost_collected_libs)
  unset(_wt_in_tree_boost)
  unset(_wt_libs)
  unset(_collected_includes)
else()
  #
  # --- No in-tree targets — fall back to CMake's find_package(Boost) ---
  #
  function(find_boost_system)
    find_package(Boost 1.66 QUIET COMPONENTS system)
    set(_BOOST_FOUND ${Boost_FOUND} PARENT_SCOPE)
    if(DEFINED Boost_VERSION_MAJOR)
      set(_BOOST_MAJOR_VERSION ${Boost_VERSION_MAJOR} PARENT_SCOPE)
      set(_BOOST_MINOR_VERSION ${Boost_VERSION_MINOR} PARENT_SCOPE)
    else()
      set(_BOOST_MAJOR_VERSION ${Boost_MAJOR_VERSION} PARENT_SCOPE)
      set(_BOOST_MINOR_VERSION ${Boost_MINOR_VERSION} PARENT_SCOPE)
    endif()
  endfunction()

  find_boost_system()

  if(_BOOST_FOUND AND _BOOST_MAJOR_VERSION EQUAL 1 AND _BOOST_MINOR_VERSION LESS 69)
    LIST(APPEND Boost_COMPONENTS system)
  endif()

  # clear temporaries
  set(_BOOST_FOUND)
  set(_BOOST_MAJOR_VERSION)
  set(_BOOST_MINOR_VERSION)

  FIND_PACKAGE(Boost 1.66 REQUIRED COMPONENTS ${Boost_COMPONENTS})

  # If find_package provided IMPORTED targets, map them to Wt vars
  foreach(_target ${Boost_COMPONENTS})
    if(TARGET Boost::${_target})
      string(TOUPPER ${_target} _TARGET_UP)
      set(Boost_${_TARGET_UP}_LIBRARY Boost::${_target})
    endif()
  endforeach()

  if(Boost_FOUND)
    if(Boost_VERSION_MAJOR AND NOT Boost_MAJOR_VERSION)
      set(Boost_MAJOR_VERSION ${Boost_VERSION_MAJOR})
    endif()
    if(Boost_VERSION_MINOR AND NOT Boost_MINOR_VERSION)
      set(Boost_MINOR_VERSION ${Boost_VERSION_MINOR})
    endif()

    # Translate official CMake vars to Wt expected names (safe guards)
    if(DEFINED Boost_INCLUDE_DIRS)
      set(BOOST_INCLUDE_DIRS ${Boost_INCLUDE_DIRS})
    endif()
    if(DEFINED Boost_LIBRARY_DIRS)
      set(BOOST_LIB_DIRS ${Boost_LIBRARY_DIRS})
    endif()
    if(DEFINED Boost_FILESYSTEM_LIBRARY)
      set(BOOST_FS_LIB ${Boost_FILESYSTEM_LIBRARY})
    endif()
    if(DEFINED Boost_PROGRAM_OPTIONS_LIBRARY)
      set(BOOST_PO_LIB ${Boost_PROGRAM_OPTIONS_LIBRARY})
    endif()
    if(DEFINED Boost_SYSTEM_LIBRARY)
      set(BOOST_SYSTEM_LIB ${Boost_SYSTEM_LIBRARY})
    endif()
    if(DEFINED Boost_THREAD_LIBRARY)
      set(BOOST_THREAD_LIB ${Boost_THREAD_LIBRARY})
    endif()

    if(WIN32 AND BOOST_DYNAMIC AND BUILD_TESTS)
      set(BOOST_UT_LIB ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
    else()
      set(BOOST_UT_LIB "")
    endif()

    set(BOOST_WT_MT_FOUND TRUE)
    set(BOOST_WT_FOUND TRUE)
    set(BOOST_WTHTTP_MT_FOUND TRUE)
    set(BOOST_WTHTTP_FOUND TRUE)

    if(MSVC AND Boost_MAJOR_VERSION EQUAL 1 AND Boost_MINOR_VERSION LESS 70)
      set(BOOST_WT_LIBRARIES "")
      set(BOOST_WTHTTP_LIBRARIES "")
      set(BOOST_TEST_LIBRARIES "")
    else()
      set(BOOST_WT_LIBRARIES
        ${Boost_THREAD_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_FILESYSTEM_LIBRARY}
      )
      set(BOOST_WTHTTP_LIBRARIES
        ${Boost_THREAD_LIBRARY}
        ${Boost_PROGRAM_OPTIONS_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_FILESYSTEM_LIBRARY}
      )
      set(BOOST_TEST_LIBRARIES ${BOOST_UT_LIB})
    endif()

    if(TARGET Boost::headers)
      set(BOOST_WT_LIBRARIES ${BOOST_WT_LIBRARIES} Boost::headers)
      set(BOOST_WTHTTP_LIBRARIES ${BOOST_WTHTTP_LIBRARIES} Boost::headers)
    endif()
  endif()
endif()


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ STARTING POINT @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@


# WtFindBoost.txt - patched safe version to support in-tree Boost targets
#
# Sets:
#   BOOST_INCLUDE_DIRS, BOOST_LIB_DIRS, BOOST_WT_LIBRARIES, BOOST_WTHTTP_LIBRARIES, ...
#
# Behaviour:
#  - If Boost::xxx targets already exist (from add_subdirectory(boost)), map them into
#    the BOOST_* variables Wt expects and skip calling find_package()
#  - Otherwise fall back to the original find_package(Boost ...) logic

# default state
SET(BOOST_WT_FOUND FALSE)
SET(BOOST_WT_MT_FOUND FALSE)
SET(BOOST_WTHTTP_FOUND FALSE)
SET(BOOST_WTHTTP_MT_FOUND FALSE)
SET(BOOST_SUPPORT_LIBRARIES "")

IF(NOT DEFINED Boost_ADDITIONAL_VERSIONS)
  SET(Boost_ADDITIONAL_VERSIONS
    "1.66" "1.67" "1.68" "1.69" "1.70" "1.71" "1.72" "1.73" "1.74" "1.75"
    "1.76" "1.77" "1.78" "1.79" "1.80" "1.81" "1.82" "1.83" "1.84" "1.85"
    "1.86" "1.87" "1.88")
ENDIF()

SET(Boost_USE_MULTITHREADED ON)

IF(DEFINED BOOST_PREFIX AND NOT DEFINED BOOST_ROOT)
  SET(BOOST_ROOT ${BOOST_PREFIX})
ENDIF()

IF(WIN32)
  IF(BOOST_DYNAMIC)
    SET(Boost_USE_STATIC_LIBS OFF)
  ELSE()
    SET(Boost_USE_STATIC_LIBS ON)
  ENDIF()
ENDIF()

SET(Boost_DEBUG ON)

# Explicit components Wt may use — added asio, chrono, filesystem, system, numeric explicitly.
SET(Boost_COMPONENTS
    program_options
    thread
    asio
    filesystem
    chrono
    system
    numeric
)

IF(WT_CPP17_FILESYSTEM_IMPLEMENTATION STREQUAL "boost")
  LIST(APPEND Boost_COMPONENTS filesystem)
ENDIF()

IF(WIN32 AND BOOST_DYNAMIC AND BUILD_TESTS)
  LIST(APPEND Boost_COMPONENTS unit_test_framework)
ENDIF()

#
# --- Detect in-tree Boost targets created by add_subdirectory(boost) ---
#
set(_wt_in_tree_boost FALSE)
set(_wt_boost_includes "")
set(_wt_boost_collected_libs "")

foreach(_comp ${Boost_COMPONENTS})
  if(TARGET Boost::${_comp})
    set(_wt_in_tree_boost TRUE)
    # collect target reference
    list(APPEND _wt_boost_collected_libs Boost::${_comp})
    # if we don't already have include dirs, try to get them from the target
    if(NOT _wt_boost_includes)
      get_target_property(_incdirs Boost::${_comp} INTERFACE_INCLUDE_DIRECTORIES)
      if(_incdirs)
        set(_wt_boost_includes "${_incdirs}")
      endif()
    endif()
  endif()
endforeach()

if(_wt_in_tree_boost)
  message(STATUS "WtFindBoost: using in-tree Boost targets (Boost::...)")

  set(Boost_FOUND TRUE CACHE INTERNAL "Found in-tree Boost")
  set(BOOST_LIB_DIRS "")

  # Prefer common Wt libs
  set(_wt_libs "")
  if(TARGET Boost::thread)
    list(APPEND _wt_libs Boost::thread)
    set(BOOST_THREAD_LIB Boost::thread)
  endif()
  if(TARGET Boost::system)
    list(APPEND _wt_libs Boost::system)
    set(BOOST_SYSTEM_LIB Boost::system)
  endif()
  if(TARGET Boost::filesystem)
    list(APPEND _wt_libs Boost::filesystem)
    set(BOOST_FS_LIB Boost::filesystem)
  endif()
  if(TARGET Boost::asio)
    list(APPEND _wt_libs Boost::asio)
    set(BOOST_ASIO_LIB Boost::asio)
  endif()
  if(TARGET Boost::chrono)
    list(APPEND _wt_libs Boost::chrono)
    set(BOOST_CHRONO_LIB Boost::chrono)
  endif()
  if(TARGET Boost::numeric)
    list(APPEND _wt_libs Boost::numeric)
    set(BOOST_NUMERIC_LIB Boost::numeric)
  endif()

  set(BOOST_WT_LIBRARIES ${_wt_libs})
  set(BOOST_WTHTTP_LIBRARIES ${_wt_libs})

  if(TARGET Boost::headers)
    list(APPEND BOOST_WT_LIBRARIES Boost::headers)
    list(APPEND BOOST_WTHTTP_LIBRARIES Boost::headers)
  endif()

  set(BOOST_WT_FOUND TRUE)
  set(BOOST_WT_MT_FOUND TRUE)
  set(BOOST_WTHTTP_FOUND TRUE)
  set(BOOST_WTHTTP_MT_FOUND TRUE)

  #
  # --- Robust collection of Boost include directories ---
  #
  set(_collected_includes "")

  if(_wt_boost_includes)
    list(APPEND _collected_includes ${_wt_boost_includes})
  endif()

  if(TARGET Boost::headers)
    get_target_property(_hdrincs Boost::headers INTERFACE_INCLUDE_DIRECTORIES)
    if(_hdrincs)
      list(APPEND _collected_includes ${_hdrincs})
    endif()
  endif()

  foreach(_bt ${_wt_boost_collected_libs})
    if(TARGET ${_bt})
      get_target_property(_incs ${_bt} INTERFACE_INCLUDE_DIRECTORIES)
      if(_incs)
        list(APPEND _collected_includes ${_incs})
      endif()
    endif()
  endforeach()

  # Use BOOST_INCLUDE_LIBRARIES if defined
  if(DEFINED BOOST_INCLUDE_LIBRARIES)
    foreach(_lib IN LISTS BOOST_INCLUDE_LIBRARIES)
      string(STRIP "${_lib}" _lib_trim)
      if(_lib_trim)
        set(_candidate "${CMAKE_SOURCE_DIR}/boost/libs/${_lib_trim}/include")
        if(EXISTS "${_candidate}")
          list(APPEND _collected_includes "${_candidate}")
        elseif(EXISTS "${_candidate}/boost")
          list(APPEND _collected_includes "${_candidate}")
        endif()
      endif()
    endforeach()
  else()
    if(EXISTS "${CMAKE_SOURCE_DIR}/boost/libs")
      file(GLOB _boost_lib_dirs RELATIVE "${CMAKE_SOURCE_DIR}/boost/libs" "${CMAKE_SOURCE_DIR}/boost/libs/*")
      foreach(_ldir ${_boost_lib_dirs})
        set(_candidate "${CMAKE_SOURCE_DIR}/boost/libs/${_ldir}/include")
        if(EXISTS "${_candidate}")
          list(APPEND _collected_includes "${_candidate}")
        endif()
      endforeach()
    endif()
  endif()

  # Explicitly add key include paths to ensure numeric and asio are covered
  list(APPEND _collected_includes
    "${CMAKE_SOURCE_DIR}/boost/libs/asio/include"
    "${CMAKE_SOURCE_DIR}/boost/libs/numeric/include"
  )

  list(APPEND _collected_includes "${CMAKE_SOURCE_DIR}/boost")
  list(REMOVE_DUPLICATES _collected_includes)
  set(BOOST_INCLUDE_DIRS ${_collected_includes})

  unset(_wt_boost_includes)
  unset(_wt_boost_collected_libs)
  unset(_wt_in_tree_boost)
  unset(_wt_libs)
  unset(_collected_includes)

else()
  #
  # --- No in-tree targets — fall back to CMake's find_package(Boost) ---
  #
  function(find_boost_system)
    find_package(Boost 1.66 QUIET COMPONENTS system)
    set(_BOOST_FOUND ${Boost_FOUND} PARENT_SCOPE)
    if(DEFINED Boost_VERSION_MAJOR)
      set(_BOOST_MAJOR_VERSION ${Boost_VERSION_MAJOR} PARENT_SCOPE)
      set(_BOOST_MINOR_VERSION ${Boost_VERSION_MINOR} PARENT_SCOPE)
    else()
      set(_BOOST_MAJOR_VERSION ${Boost_MAJOR_VERSION} PARENT_SCOPE)
      set(_BOOST_MINOR_VERSION ${Boost_MINOR_VERSION} PARENT_SCOPE)
    endif()
  endfunction()

  find_boost_system()

  if(_BOOST_FOUND AND _BOOST_MAJOR_VERSION EQUAL 1 AND _BOOST_MINOR_VERSION LESS 69)
    LIST(APPEND Boost_COMPONENTS system)
  endif()

  set(_BOOST_FOUND)
  set(_BOOST_MAJOR_VERSION)
  set(_BOOST_MINOR_VERSION)

  FIND_PACKAGE(Boost 1.66 REQUIRED COMPONENTS ${Boost_COMPONENTS})

  foreach(_target ${Boost_COMPONENTS})
    if(TARGET Boost::${_target})
      string(TOUPPER ${_target} _TARGET_UP)
      set(Boost_${_TARGET_UP}_LIBRARY Boost::${_target})
    endif()
  endforeach()

  if(Boost_FOUND)
    if(Boost_VERSION_MAJOR AND NOT Boost_MAJOR_VERSION)
      set(Boost_MAJOR_VERSION ${Boost_VERSION_MAJOR})
    endif()
    if(Boost_VERSION_MINOR AND NOT Boost_MINOR_VERSION)
      set(Boost_MINOR_VERSION ${Boost_VERSION_MINOR})
    endif()

    if(DEFINED Boost_INCLUDE_DIRS)
      set(BOOST_INCLUDE_DIRS ${Boost_INCLUDE_DIRS})
    endif()
    if(DEFINED Boost_LIBRARY_DIRS)
      set(BOOST_LIB_DIRS ${Boost_LIBRARY_DIRS})
    endif()
    if(DEFINED Boost_FILESYSTEM_LIBRARY)
      set(BOOST_FS_LIB ${Boost_FILESYSTEM_LIBRARY})
    endif()
    if(DEFINED Boost_PROGRAM_OPTIONS_LIBRARY)
      set(BOOST_PO_LIB ${Boost_PROGRAM_OPTIONS_LIBRARY})
    endif()
    if(DEFINED Boost_SYSTEM_LIBRARY)
      set(BOOST_SYSTEM_LIB ${Boost_SYSTEM_LIBRARY})
    endif()
    if(DEFINED Boost_THREAD_LIBRARY)
      set(BOOST_THREAD_LIB ${Boost_THREAD_LIBRARY})
    endif()

    if(WIN32 AND BOOST_DYNAMIC AND BUILD_TESTS)
      set(BOOST_UT_LIB ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
    else()
      set(BOOST_UT_LIB "")
    endif()

    set(BOOST_WT_MT_FOUND TRUE)
    set(BOOST_WT_FOUND TRUE)
    set(BOOST_WTHTTP_MT_FOUND TRUE)
    set(BOOST_WTHTTP_FOUND TRUE)

    if(MSVC AND Boost_MAJOR_VERSION EQUAL 1 AND Boost_MINOR_VERSION LESS 70)
      set(BOOST_WT_LIBRARIES "")
      set(BOOST_WTHTTP_LIBRARIES "")
      set(BOOST_TEST_LIBRARIES "")
    else()
      set(BOOST_WT_LIBRARIES
        ${Boost_THREAD_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_FILESYSTEM_LIBRARY}
      )
      set(BOOST_WTHTTP_LIBRARIES
        ${Boost_THREAD_LIBRARY}
        ${Boost_PROGRAM_OPTIONS_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_FILESYSTEM_LIBRARY}
      )
      set(BOOST_TEST_LIBRARIES ${BOOST_UT_LIB})
    endif()

    if(TARGET Boost::headers)
      set(BOOST_WT_LIBRARIES ${BOOST_WT_LIBRARIES} Boost::headers)
      set(BOOST_WTHTTP_LIBRARIES ${BOOST_WTHTTP_LIBRARIES} Boost::headers)
    endif()
  endif()
endif()

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ PROGRESS ONE @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@



# WtFindBoost.txt - patched: in-tree Boost detection + filesystem-first include discovery
#
# Behavior:
#  - If Boost::xxx targets already exist (from add_subdirectory(boost)), map them into
#  the BOOST_* variables Wt expects and skip calling find_package()
#  - Additionally, perform a filesystem-first scan for Boost include dirs for modules
#  listed in Boost_COMPONENTS (and BOOST_INCLUDE_LIBRARIES if present).
#  - Ignores generator expressions from target properties and only appends absolute,
#  existing directories to BOOST_INCLUDE_DIRS.
#  - Otherwise fall back to the original find_package(Boost ...) logic.
#
# Based on original WtFindBoost.txt (user-provided). See embedded diagnostics when needed.
#

# default state
SET(BOOST_WT_FOUND FALSE)
SET(BOOST_WT_MT_FOUND FALSE)
SET(BOOST_WTHTTP_FOUND FALSE)
SET(BOOST_WTHTTP_MT_FOUND FALSE)
SET(BOOST_SUPPORT_LIBRARIES "")

IF(NOT DEFINED Boost_ADDITIONAL_VERSIONS)
  SET(Boost_ADDITIONAL_VERSIONS
    "1.66" "1.67" "1.68" "1.69" "1.70" "1.71" "1.72" "1.73" "1.74" "1.75"
    "1.76" "1.77" "1.78" "1.79" "1.80" "1.81" "1.82" "1.83" "1.84" "1.85"
    "1.86" "1.87" "1.88")
ENDIF()

SET(Boost_USE_MULTITHREADED ON)

IF(DEFINED BOOST_PREFIX AND NOT DEFINED BOOST_ROOT)
  SET(BOOST_ROOT ${BOOST_PREFIX})
ENDIF()

IF(WIN32)
  IF(BOOST_DYNAMIC)
    SET(Boost_USE_STATIC_LIBS OFF)
  ELSE()
    SET(Boost_USE_STATIC_LIBS ON)
  ENDIF()
ENDIF()

SET(Boost_DEBUG ON)

# Default components Wt needs (adjust as required)
SET(Boost_COMPONENTS
    program_options
    thread
    asio
    filesystem
    chrono
    system
    numeric
)

IF(WT_CPP17_FILESYSTEM_IMPLEMENTATION STREQUAL "boost")
  LIST(APPEND Boost_COMPONENTS filesystem)
ENDIF()

IF(WIN32 AND BOOST_DYNAMIC AND BUILD_TESTS)
  LIST(APPEND Boost_COMPONENTS unit_test_framework)
ENDIF()

#
# --------- NEW: Detect an in-tree Boost (targets created by add_subdirectory) ---------
#
set(_wt_in_tree_boost FALSE)
set(_wt_boost_interface_includes "")  # keep generator-expr includes if we need them (we won't rely on them)
set(_wt_boost_collected_libs "")

foreach(_comp ${Boost_COMPONENTS})
  if(TARGET Boost::${_comp})
    set(_wt_in_tree_boost TRUE)
    list(APPEND _wt_boost_collected_libs Boost::${_comp})
    # collect INTERFACE_INCLUDE_DIRECTORIES but do not rely on generator expressions:
    get_target_property(_incdirs Boost::${_comp} INTERFACE_INCLUDE_DIRECTORIES)
    if(_incdirs)
      list(APPEND _wt_boost_interface_includes ${_incdirs})
    endif()
  endif()
endforeach()

if(_wt_in_tree_boost)
  message(STATUS "WtFindBoost: Boost:: targets detected (in-tree Boost). Using filesystem-first include discovery.")

  set(Boost_FOUND TRUE CACHE INTERNAL "Found in-tree Boost")
  set(BOOST_LIB_DIRS "")

  # Map commonly-used Boost::<lib> targets into Wt variables if present
  set(_wt_libs "")
  foreach(_lib program_options thread system filesystem asio chrono numeric)
    if(TARGET Boost::${_lib})
      list(APPEND _wt_libs Boost::${_lib})
      string(TOUPPER ${_lib} _ULIB)
      set(BOOST_${_ULIB}_LIB Boost::${_lib})
    endif()
  endforeach()

  set(BOOST_WT_LIBRARIES ${_wt_libs})
  set(BOOST_WTHTTP_LIBRARIES ${_wt_libs})

  if(TARGET Boost::headers)
    list(APPEND BOOST_WT_LIBRARIES Boost::headers)
    list(APPEND BOOST_WTHTTP_LIBRARIES Boost::headers)
  endif()

  set(BOOST_WT_FOUND TRUE)
  set(BOOST_WT_MT_FOUND TRUE)
  set(BOOST_WTHTTP_FOUND TRUE)
  set(BOOST_WTHTTP_MT_FOUND TRUE)

  #
  # --------- Filesystem-first include discovery (component-limited) ----------
  #
  # Goal: for each component in Boost_COMPONENTS and any in BOOST_INCLUDE_LIBRARIES,
  # collect absolute include directories that actually exist on disk:
  #   - <module>/include
  #   - <module>/*/include
  #   - <module>/<child>   (if child contains headers directly)
  #   - header-parent dirs for header files found under module
  #
  set(_collected_includes_raw "")

  # Modules to scan: union of Boost_COMPONENTS and BOOST_INCLUDE_LIBRARIES
  set(_mods "")
  if(DEFINED BOOST_INCLUDE_LIBRARIES)
    list(APPEND _mods ${BOOST_INCLUDE_LIBRARIES})
  endif()
  list(APPEND _mods ${Boost_COMPONENTS})
  list(REMOVE_DUPLICATES _mods)

  # ---------------------------------------------------------------------------
  # Helper function: collect_module_includes
  # Adds module/include, finds **/include recursively, finds header-parent dirs
  # for .hpp/.h/.ipp/.inl recursively, and includes immediate child folders that
  # contain headers (non-recursive check). Returns absolute, existing dirs only.
  function(collect_module_includes _moddir _out_var)
    if(NOT _moddir)
      set(${_out_var} "" PARENT_SCOPE)
      return()
    endif()

    set(_raw_list "")

    # 1) module-level include folder
    if(EXISTS "${_moddir}/include" AND IS_DIRECTORY "${_moddir}/include")
      list(APPEND _raw_list "${_moddir}/include")
    endif()

    # 2) find any nested .../include directories (recursive)
    file(GLOB_RECURSE _found_inc_dirs LIST_DIRECTORIES true "${_moddir}/**/include")
    foreach(_d IN LISTS _found_inc_dirs)
      if(IS_DIRECTORY "${_d}")
        list(APPEND _raw_list "${_d}")
      endif()
    endforeach()

    # 3) collect header-parent dirs for headers found under module (recursive)
    file(GLOB_RECURSE _found_headers
         "${_moddir}/**/*.hpp"
         "${_moddir}/**/*.h"
         "${_moddir}/**/*.ipp"
         "${_moddir}/**/*.inl")
    foreach(_hdr IN LISTS _found_headers)
      get_filename_component(_hdr_dir "${_hdr}" DIRECTORY)
      if(_hdr_dir AND IS_DIRECTORY "${_hdr_dir}")
        list(APPEND _raw_list "${_hdr_dir}")
      endif()
    endforeach()

    # 4) immediate child folders of _moddir that contain headers (non-recursive)
    file(GLOB _children LIST_DIRECTORIES true "${_moddir}/*")
    foreach(_child IN LISTS _children)
      if(IS_DIRECTORY "${_child}")
        # check for headers directly in this child (non-recursive)
        file(GLOB _child_hdrs1 "${_child}/*.hpp")
        file(GLOB _child_hdrs2 "${_child}/*.h")
        file(GLOB _child_hdrs3 "${_child}/*.ipp")
        file(GLOB _child_hdrs4 "${_child}/*.inl")
        if(_child_hdrs1 OR _child_hdrs2 OR _child_hdrs3 OR _child_hdrs4)
          list(APPEND _raw_list "${_child}")
        endif()
        # also include child/include if it exists (covers child/include layouts)
        if(EXISTS "${_child}/include" AND IS_DIRECTORY "${_child}/include")
          list(APPEND _raw_list "${_child}/include")
        endif()
      endif()
    endforeach()

    # 5) Deduplicate and keep only absolute, existing directories; skip generator expressions
    list(REMOVE_DUPLICATES _raw_list)

    set(_final_list "")
    foreach(_p IN LISTS _raw_list)
      if(NOT _p)
        continue()
      endif()
      if("${_p}" MATCHES "\\$<")
        # contains generator-expression; skip from filesystem-first list
        continue()
      endif()

      get_filename_component(_abs "${_p}" ABSOLUTE)
      if(IS_DIRECTORY "${_abs}")
        list(APPEND _final_list "${_abs}")
      else()
        if(EXISTS "${_p}" AND IS_DIRECTORY "${_p}")
          get_filename_component(_abs2 "${_p}" REALPATH)
          if(_abs2 AND IS_DIRECTORY "${_abs2}")
            list(APPEND _final_list "${_abs2}")
          endif()
        endif()
      endif()
    endforeach()

    list(REMOVE_DUPLICATES _final_list)
    set(${_out_var} "${_final_list}" PARENT_SCOPE)
  endfunction()
  # ---------------------------------------------------------------------------

  foreach(_mod IN LISTS _mods)
    string(STRIP "${_mod}" _mod_trim)
    if(NOT _mod_trim)
      continue()
    endif()

    # Prefer BOOST_ROOT or fall back to project source dirs
    if(DEFINED BOOST_ROOT)
      set(_moddir "${BOOST_ROOT}/libs/${_mod_trim}")
    else()
      set(_moddir "${CMAKE_SOURCE_DIR}/boost/libs/${_mod_trim}")
    endif()

    # If moddir missing, also try current source dir path (some projects configure from subdir)
    if(NOT EXISTS "${_moddir}")
      set(_moddir_alt "${CMAKE_CURRENT_SOURCE_DIR}/boost/libs/${_mod_trim}")
      if(EXISTS "${_moddir_alt}")
        set(_moddir "${_moddir_alt}")
      endif()
    endif()

    if(NOT EXISTS "${_moddir}")
      # module not present in source tree - skip
      message(STATUS "WtFindBoost: module not present, skipping: ${_mod_trim}")
      continue()
    endif()

    collect_module_includes("${_moddir}" _module_incs)
    foreach(_inc IN LISTS _module_incs)
      list(APPEND _collected_includes_raw "${_inc}")
    endforeach()

  endforeach() # _mods

  # (5) Always append top-level boost source dir as fallback (use BOOST_ROOT if set)
  if(DEFINED BOOST_ROOT)
    list(APPEND _collected_includes_raw "${BOOST_ROOT}")
  else()
    list(APPEND _collected_includes_raw "${CMAKE_SOURCE_DIR}/boost")
  endif()

  # (6) Deduplicate and keep only existing absolute directories (ignore generator expressions)
  list(REMOVE_DUPLICATES _collected_includes_raw)
  set(_final_includes "")
  foreach(_p IN LISTS _collected_includes_raw)
    # Normalize and skip if looks like generator-expression or not a path
    if("${_p}" MATCHES "\\$<.*:.*>") # skip generator expressions
      continue()
    endif()
    if(_p AND IS_DIRECTORY "${_p}")
      get_filename_component(_abs_p "${_p}" ABSOLUTE)
      list(APPEND _final_includes "${_abs_p}")
    endif()
  endforeach()

  # Export as BOOST_INCLUDE_DIRS (CACHE INTERNAL to avoid accidental override)
  set(BOOST_INCLUDE_DIRS ${_final_includes} CACHE INTERNAL "Collected in-tree Boost include dirs")

  # Debug summary: report what we discovered (helpful for verification)
  list(LENGTH _final_includes _len)
  message(STATUS "WtFindBoost: discovered ${_len} physical include directories under boost/libs (showing up to 50):")
  if(_len GREATER 0)
    if(_len LESS 50)
      set(_max_show ${_len})
    else()
      set(_max_show 50)
    endif()
    foreach(_i RANGE 0 ${_max_show})
      if(_i LESS _len)
        list(GET _final_includes ${_i} _show)
        message(STATUS "  ${_show}")
      endif()
    endforeach()
  else()
    message(STATUS "  (no physical include directories discovered via filesystem scan)")
    # If nothing found, as a diagnostic step, also output interface include props (generator-expr) if any
    if(_wt_boost_interface_includes)
      message(STATUS "  Note: Boost targets expose INTERFACE include properties (generator-expr); showing them for debugging:")
      foreach(_gi IN LISTS _wt_boost_interface_includes)
        message(STATUS "    ${_gi}")
      endforeach()
    endif()
  endif()

  # Done: set other expected values (some were set earlier)
  set(BOOST_WT_FOUND TRUE)
  set(BOOST_WT_MT_FOUND TRUE)
  set(BOOST_WTHTTP_FOUND TRUE)
  set(BOOST_WTHTTP_MT_FOUND TRUE)

  # Cleanup temporaries
  unset(_collected_includes_raw)
  unset(_found_inc_dirs)
  unset(_found_headers)
  unset(_child_dirs)
  unset(_child_hdrs)
  unset(_final_includes)
  unset(_mods)
  unset(_wt_boost_interface_includes)
  unset(_wt_boost_collected_libs)
  unset(_wt_in_tree_boost)
  unset(_wt_libs)

#
# --------- END in-tree detection and filesystem discovery -------------------
#
else()
  #
  # --- No in-tree targets — fall back to CMake's find_package(Boost) ---
  #
  function(find_boost_system)
    find_package(Boost 1.66 QUIET
      COMPONENTS
        system
    )

    set(_BOOST_FOUND ${Boost_FOUND} PARENT_SCOPE)
    if(Boost_VERSION_MAJOR)
      set(_BOOST_MAJOR_VERSION ${Boost_VERSION_MAJOR} PARENT_SCOPE)
      set(_BOOST_MINOR_VERSION ${Boost_VERSION_MINOR} PARENT_SCOPE)
    else()
      set(_BOOST_MAJOR_VERSION ${Boost_MAJOR_VERSION} PARENT_SCOPE)
      set(_BOOST_MINOR_VERSION ${Boost_MINOR_VERSION} PARENT_SCOPE)
    endif()
  endfunction()

  find_boost_system()

  if(_BOOST_FOUND AND _BOOST_MAJOR_VERSION EQUAL 1 AND _BOOST_MINOR_VERSION LESS 69)
    LIST(APPEND Boost_COMPONENTS system)
  endif()

  set(_BOOST_FOUND)
  set(_BOOST_MAJOR_VERSION)
  set(_BOOST_MINOR_VERSION)

  FIND_PACKAGE(Boost 1.66
    COMPONENTS
      ${Boost_COMPONENTS}
    REQUIRED
  )

  # Use IMPORTED targets if defined (map to Wt variables)
  FOREACH(_target ${Boost_COMPONENTS})
    IF(TARGET Boost::${_target})
      STRING(TOUPPER ${_target} _TARGET)
      SET(Boost_${_TARGET}_LIBRARY Boost::${_target})
    ENDIF()
  ENDFOREACH()
  UNSET(_target)
  UNSET(_TARGET)

  IF (Boost_FOUND)
    IF(Boost_VERSION_MAJOR AND NOT Boost_MAJOR_VERSION)
      set(Boost_MAJOR_VERSION ${Boost_VERSION_MAJOR})
    ENDIF()
    IF(Boost_VERSION_MINOR AND NOT Boost_MINOR_VERSION)
      set(Boost_MINOR_VERSION ${Boost_VERSION_MINOR})
    ENDIF()

    # Translate variables from official cmake names to internally used names
    IF(NOT DEFINED BOOST_INCLUDE_DIRS AND DEFINED Boost_INCLUDE_DIRS)
      SET(BOOST_INCLUDE_DIRS ${Boost_INCLUDE_DIRS})
    ENDIF()
    IF(NOT DEFINED BOOST_LIB_DIRS AND DEFINED Boost_LIBRARY_DIRS)
      SET(BOOST_LIB_DIRS ${Boost_LIBRARY_DIRS})
    ENDIF()

    # If find_package actually set variables for individual libs, map them:
    if(DEFINED Boost_FILESYSTEM_LIBRARY)
      SET(BOOST_FS_LIB ${Boost_FILESYSTEM_LIBRARY})
    endif()
    if(DEFINED Boost_PROGRAM_OPTIONS_LIBRARY)
      SET(BOOST_PO_LIB ${Boost_PROGRAM_OPTIONS_LIBRARY})
    endif()
    if(DEFINED Boost_SYSTEM_LIBRARY)
      SET(BOOST_SYSTEM_LIB ${Boost_SYSTEM_LIBRARY})
    endif()
    if(DEFINED Boost_THREAD_LIBRARY)
      SET(BOOST_THREAD_LIB ${Boost_THREAD_LIBRARY})
    endif()
    IF(WIN32 AND BOOST_DYNAMIC AND BUILD_TESTS)
      SET(BOOST_UT_LIB ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
    ELSE()
      IF(NOT DEFINED BOOST_UT_LIB)
        SET(BOOST_UT_LIB "")
      ENDIF()
    ENDIF()

    SET(BOOST_WT_MT_FOUND TRUE)
    SET(BOOST_WT_FOUND TRUE)
    SET(BOOST_WTHTTP_MT_FOUND TRUE)
    SET(BOOST_WTHTTP_FOUND TRUE)

    IF(MSVC AND Boost_MAJOR_VERSION EQUAL 1 AND Boost_MINOR_VERSION LESS 70)
      # use autolink
      SET(BOOST_WT_LIBRARIES "")
      SET(BOOST_WTHTTP_LIBRARIES "")
      SET(BOOST_TEST_LIBRARIES "")
    ELSE()
      # prefer explicit variables if available, otherwise rely on the Boost::<lib> targets we set above
      SET(BOOST_WT_LIBRARIES
          ${Boost_THREAD_LIBRARY}
          ${Boost_SYSTEM_LIBRARY}
          ${Boost_FILESYSTEM_LIBRARY})
      SET(BOOST_WTHTTP_LIBRARIES
          ${Boost_THREAD_LIBRARY}
          ${Boost_PROGRAM_OPTIONS_LIBRARY}
          ${Boost_SYSTEM_LIBRARY}
          ${Boost_FILESYSTEM_LIBRARY})
      SET(BOOST_TEST_LIBRARIES ${BOOST_UT_LIB})
    ENDIF()

    IF(TARGET Boost::headers)
      SET(BOOST_WT_LIBRARIES ${BOOST_WT_LIBRARIES} Boost::headers)
      SET(BOOST_WTHTTP_LIBRARIES ${BOOST_WTHTTP_LIBRARIES} Boost::headers)
    ENDIF()
  ENDIF()
endif()


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ PROGRESS ONE @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@



# WtFindBoost.cmake - patched: in-tree Boost detection + filesystem-first include discovery
#
# Behavior:
#  - If Boost::xxx targets already exist (from add_subdirectory(boost)), map them into
#    the BOOST_* variables Wt expects and skip calling find_package()
#  - Additionally, perform a filesystem-first scan for Boost include dirs for modules
#    listed in Boost_COMPONENTS (and BOOST_INCLUDE_LIBRARIES if present).
#  - Ignores generator expressions from target properties and only appends absolute,
#    existing directories to BOOST_INCLUDE_DIRS.
#  - Otherwise fall back to the original find_package(Boost ...) logic.
#
# Based on original WtFindBoost.txt (user-provided). See embedded diagnostics when needed.
#

# default state
SET(BOOST_WT_FOUND FALSE)
SET(BOOST_WT_MT_FOUND FALSE)
SET(BOOST_WTHTTP_FOUND FALSE)
SET(BOOST_WTHTTP_MT_FOUND FALSE)
SET(BOOST_SUPPORT_LIBRARIES "")

IF(NOT DEFINED Boost_ADDITIONAL_VERSIONS)
  SET(Boost_ADDITIONAL_VERSIONS
    "1.66" "1.67" "1.68" "1.69" "1.70" "1.71" "1.72" "1.73" "1.74" "1.75"
    "1.76" "1.77" "1.78" "1.79" "1.80" "1.81" "1.82" "1.83" "1.84" "1.85"
    "1.86" "1.87" "1.88")
ENDIF()

SET(Boost_USE_MULTITHREADED ON)

IF(DEFINED BOOST_PREFIX AND NOT DEFINED BOOST_ROOT)
  SET(BOOST_ROOT ${BOOST_PREFIX})
ENDIF()

IF(WIN32)
  IF(BOOST_DYNAMIC)
    SET(Boost_USE_STATIC_LIBS OFF)
  ELSE()
    SET(Boost_USE_STATIC_LIBS ON)
  ENDIF()
ENDIF()

SET(Boost_DEBUG ON)

# Default components Wt needs (adjust as required)
SET(Boost_COMPONENTS
    program_options
    thread
    asio
    filesystem
    chrono
    system
    numeric
)

IF(WT_CPP17_FILESYSTEM_IMPLEMENTATION STREQUAL "boost")
  LIST(APPEND Boost_COMPONENTS filesystem)
ENDIF()

IF(WIN32 AND BOOST_DYNAMIC AND BUILD_TESTS)
  LIST(APPEND Boost_COMPONENTS unit_test_framework)
ENDIF()

#
# --------- NEW: Detect an in-tree Boost (targets created by add_subdirectory) ---------
#
set(_wt_in_tree_boost FALSE)
set(_wt_boost_interface_includes "")  # keep generator-expr includes if we need them (we won't rely on them)
set(_wt_boost_collected_libs "")

foreach(_comp ${Boost_COMPONENTS})
  if(TARGET Boost::${_comp})
    set(_wt_in_tree_boost TRUE)
    list(APPEND _wt_boost_collected_libs Boost::${_comp})
    # collect INTERFACE_INCLUDE_DIRECTORIES but do not rely on generator expressions:
    get_target_property(_incdirs Boost::${_comp} INTERFACE_INCLUDE_DIRECTORIES)
    if(_incdirs)
      list(APPEND _wt_boost_interface_includes ${_incdirs})
    endif()
  endif()
endforeach()

if(_wt_in_tree_boost)
  message(STATUS "WtFindBoost: Boost:: targets detected (in-tree Boost). Using filesystem-first include discovery.")

  set(Boost_FOUND TRUE CACHE INTERNAL "Found in-tree Boost")
  set(BOOST_LIB_DIRS "")

  # Map commonly-used Boost::<lib> targets into Wt variables if present
  set(_wt_libs "")
  foreach(_lib program_options thread system filesystem asio chrono numeric)
    if(TARGET Boost::${_lib})
      list(APPEND _wt_libs Boost::${_lib})
      string(TOUPPER ${_lib} _ULIB)
      set(BOOST_${_ULIB}_LIB Boost::${_lib})
    endif()
  endforeach()

  set(BOOST_WT_LIBRARIES ${_wt_libs})
  set(BOOST_WTHTTP_LIBRARIES ${_wt_libs})

  if(TARGET Boost::headers)
    list(APPEND BOOST_WT_LIBRARIES Boost::headers)
    list(APPEND BOOST_WTHTTP_LIBRARIES Boost::headers)
  endif()

  set(BOOST_WT_FOUND TRUE)
  set(BOOST_WT_MT_FOUND TRUE)
  set(BOOST_WTHTTP_FOUND TRUE)
  set(BOOST_WTHTTP_MT_FOUND TRUE)

  #
  # --------- Filesystem-first include discovery (component-limited) ----------
  #
  # Goal: for each component in Boost_COMPONENTS and any in BOOST_INCLUDE_LIBRARIES,
  # collect absolute include directories that actually exist on disk:
  #   - <module>/include
  #   - <module>/*/include
  #   - <module>/<child>   (if child contains headers directly)
  #   - header-parent dirs for header files found under module
  #

  set(_collected_includes_raw "")

  # Modules to scan: union of Boost_COMPONENTS and BOOST_INCLUDE_LIBRARIES
  set(_mods "")
  if(DEFINED BOOST_INCLUDE_LIBRARIES)
    list(APPEND _mods ${BOOST_INCLUDE_LIBRARIES})
  endif()
  list(APPEND _mods ${Boost_COMPONENTS})
  list(REMOVE_DUPLICATES _mods)

  # ---------------------------------------------------------------------------
  # Improved helper function: collect_module_includes
  # - Accepts module include layouts
  # - Keeps only directories that are 'include' dirs or contain headers (direct)
  # - Filters out noisy paths (docs, tests, examples, .github, etc.)
  function(collect_module_includes _moddir _out_var)
    if(NOT _moddir)
      set(${_out_var} "" PARENT_SCOPE)
      return()
    endif()

    set(_raw_list "")

    # 1) module-level include folder
    if(EXISTS "${_moddir}/include" AND IS_DIRECTORY "${_moddir}/include")
      list(APPEND _raw_list "${_moddir}/include")
    endif()

    # 2) find any nested .../include directories (recursive)
    file(GLOB_RECURSE _found_inc_dirs LIST_DIRECTORIES true "${_moddir}/**/include")
    foreach(_d IN LISTS _found_inc_dirs)
      if(IS_DIRECTORY "${_d}")
        list(APPEND _raw_list "${_d}")
      endif()
    endforeach()

    # 3) collect header-parent dirs for headers found under module (recursive)
    file(GLOB_RECURSE _found_headers
         "${_moddir}/**/*.hpp"
         "${_moddir}/**/*.h"
         "${_moddir}/**/*.ipp"
         "${_moddir}/**/*.inl")
    foreach(_hdr IN LISTS _found_headers)
      get_filename_component(_hdr_dir "${_hdr}" DIRECTORY)
      if(_hdr_dir AND IS_DIRECTORY "${_hdr_dir}")
        list(APPEND _raw_list "${_hdr_dir}")
      endif()
    endforeach()

    # 4) immediate child folders of _moddir that contain headers (non-recursive)
    file(GLOB _children LIST_DIRECTORIES true "${_moddir}/*")
    foreach(_child IN LISTS _children)
      if(IS_DIRECTORY "${_child}")
        # include child/include if present
        if(EXISTS "${_child}/include" AND IS_DIRECTORY "${_child}/include")
          list(APPEND _raw_list "${_child}/include")
        endif()
        # add child if it directly contains headers (non-recursive)
        file(GLOB _child_hdrs "${_child}/*.{hpp,h,ipp,inl}")
        if(_child_hdrs)
          list(APPEND _raw_list "${_child}")
        endif()
      endif()
    endforeach()

    # dedupe
    list(REMOVE_DUPLICATES _raw_list)

    # noise filter regex (skip these common non-include folders)
    # anchored to any path segment; common noise folders are skipped
    set(_noise_regex "/(doc|docs|example|examples|test|tests|bench|benchmark|benchmarks|\\.github|cmake_subdir_test|\\.git)/")

    set(_final_list "")
    foreach(_p IN LISTS _raw_list)
      if(NOT _p)
        continue()
      endif()
      # skip generator expressions or weird tokens
      if("${_p}" MATCHES "\\$<")
        continue()
      endif()

      # normalize
      get_filename_component(_abs_p "${_p}" ABSOLUTE)

      # quick noise exclusion by path pattern
      if("${_abs_p}" MATCHES "${_noise_regex}")
        continue()
      endif()

      # Accept if directory name is 'include'
      get_filename_component(_basename "${_abs_p}" NAME)
      if(_basename STREQUAL "include")
        if(IS_DIRECTORY "${_abs_p}")
          list(APPEND _final_list "${_abs_p}")
          continue()
        endif()
      endif()

      # Accept if directory contains header files directly (non recursive)
      file(GLOB _hdrs_direct "${_abs_p}/*.{hpp,h,ipp,inl}")
      if(_hdrs_direct)
        list(APPEND _final_list "${_abs_p}")
        continue()
      endif()

      # Accept if immediate children contain header files (one level deep)
      file(GLOB _immediate_children LIST_DIRECTORIES true "${_abs_p}/*")
      set(_accepted FALSE)
      foreach(_ch IN LISTS _immediate_children)
        if(IS_DIRECTORY "${_ch}")
          file(GLOB _child_hdrs "${_ch}/*.{hpp,h,ipp,inl}")
          if(_child_hdrs)
            set(_accepted TRUE)
            break()
          endif()
        endif()
      endforeach()
      if(_accepted)
        list(APPEND _final_list "${_abs_p}")
        continue()
      endif()

      # otherwise skip
    endforeach()

    # final dedupe + sanity filter (ensure exists) and resolve REALPATH
    list(REMOVE_DUPLICATES _final_list)
    set(_out_list "")
    foreach(_q IN LISTS _final_list)
      if(IS_DIRECTORY "${_q}")
        get_filename_component(_realq "${_q}" REALPATH)
        if(_realq AND IS_DIRECTORY "${_realq}")
          list(APPEND _out_list "${_realq}")
        endif()
      endif()
    endforeach()

    list(REMOVE_DUPLICATES _out_list)
    set(${_out_var} "${_out_list}" PARENT_SCOPE)
  endfunction()
  # ---------------------------------------------------------------------------

  foreach(_mod IN LISTS _mods)
    string(STRIP "${_mod}" _mod_trim)
    if(NOT _mod_trim)
      continue()
    endif()

    # Prefer BOOST_ROOT or fall back to project source dirs
    if(DEFINED BOOST_ROOT)
      set(_moddir "${BOOST_ROOT}/libs/${_mod_trim}")
    else()
      set(_moddir "${CMAKE_SOURCE_DIR}/boost/libs/${_mod_trim}")
    endif()

    # If moddir missing, also try current source dir path (some projects configure from subdir)
    if(NOT EXISTS "${_moddir}")
      set(_moddir_alt "${CMAKE_CURRENT_SOURCE_DIR}/boost/libs/${_mod_trim}")
      if(EXISTS "${_moddir_alt}")
        set(_moddir "${_moddir_alt}")
      endif()
    endif()

    if(NOT EXISTS "${_moddir}")
      # module not present in source tree - skip
      message(STATUS "WtFindBoost: module not present, skipping: ${_mod_trim}")
      continue()
    endif()

    collect_module_includes("${_moddir}" _module_incs)
    foreach(_inc IN LISTS _module_incs)
      list(APPEND _collected_includes_raw "${_inc}")
    endforeach()

  endforeach() # _mods

  # Always append top-level boost source dir as fallback (use BOOST_ROOT if set)
  if(DEFINED BOOST_ROOT)
    list(APPEND _collected_includes_raw "${BOOST_ROOT}")
  else()
    list(APPEND _collected_includes_raw "${CMAKE_SOURCE_DIR}/boost")
  endif()

  # Deduplicate collected raw candidates
  list(REMOVE_DUPLICATES _collected_includes_raw)

  # Final pass: run collect_module_includes on any module-root-like paths to ensure sanity
  set(_kept "")
  foreach(_p IN LISTS _collected_includes_raw)
    if(NOT _p)
      continue()
    endif()

    # skip generator-expressions
    if("${_p}" MATCHES "\\$<.*:.*>")
      continue()
    endif()

    # If this path is a module root (e.g. .../boost/libs/<mod>) then run helper on it
    if(IS_DIRECTORY "${_p}")
      collect_module_includes("${_p}" _tmpincs)
      foreach(_i IN LISTS _tmpincs)
        list(APPEND _kept "${_i}")
      endforeach()
    endif()
  endforeach()

  list(REMOVE_DUPLICATES _kept)

  # Export as BOOST_INCLUDE_DIRS (CACHE INTERNAL to avoid accidental override)
  set(BOOST_INCLUDE_DIRS ${_kept} CACHE INTERNAL "Collected in-tree Boost include dirs (filtered)")

  # Add to global include path for the configure/build process.
  # If you prefer target-level adding, replace this with target_include_directories calls.
  if(BOOST_INCLUDE_DIRS)
    include_directories(${BOOST_INCLUDE_DIRS})
    foreach(_inc IN LISTS BOOST_INCLUDE_DIRS)
      message(STATUS "WtFindBoost: adding include dir => ${_inc}")
    endforeach()
  else()
    message(STATUS "WtFindBoost: no filtered include dirs to add")
    # If nothing found, as a diagnostic step, also output interface include props (generator-expr) if any
    if(_wt_boost_interface_includes)
      message(STATUS "  Note: Boost targets expose INTERFACE include properties (generator-expr); showing them for debugging:")
      foreach(_gi IN LISTS _wt_boost_interface_includes)
        message(STATUS "    ${_gi}")
      endforeach()
    endif()
  endif()

  # Debug summary: report what we discovered originally (helpful for verification)
  list(LENGTH _collected_includes_raw _len)
  message(STATUS "WtFindBoost: discovered ${_len} physical include directories under boost/libs (pre-filter, showing up to 50):")
  if(_len GREATER 0)
    if(_len LESS 50)
      set(_max_show ${_len})
    else()
      set(_max_show 50)
    endif()
    foreach(_i RANGE 0 ${_max_show})
      if(_i LESS _len)
        list(GET _collected_includes_raw ${_i} _show)
        message(STATUS "  ${_show}")
      endif()
    endforeach()
  endif()

  # Done: set other expected values (some were set earlier)
  set(BOOST_WT_FOUND TRUE)
  set(BOOST_WT_MT_FOUND TRUE)
  set(BOOST_WTHTTP_FOUND TRUE)
  set(BOOST_WTHTTP_MT_FOUND TRUE)

  # Cleanup temporaries
  unset(_collected_includes_raw)
  unset(_found_inc_dirs)
  unset(_found_headers)
  unset(_child_dirs)
  unset(_child_hdrs)
  unset(_final_includes)
  unset(_mods)
  unset(_wt_boost_interface_includes)
  unset(_wt_boost_collected_libs)
  unset(_wt_in_tree_boost)
  unset(_wt_libs)

#
# --------- END in-tree detection and filesystem discovery -------------------
#
else()
  #
  # --- No in-tree targets — fall back to CMake's find_package(Boost) ---
  #
  function(find_boost_system)
    find_package(Boost 1.66 QUIET
      COMPONENTS
        system
    )

    set(_BOOST_FOUND ${Boost_FOUND} PARENT_SCOPE)
    if(Boost_VERSION_MAJOR)
      set(_BOOST_MAJOR_VERSION ${Boost_VERSION_MAJOR} PARENT_SCOPE)
      set(_BOOST_MINOR_VERSION ${Boost_VERSION_MINOR} PARENT_SCOPE)
    else()
      set(_BOOST_MAJOR_VERSION ${Boost_MAJOR_VERSION} PARENT_SCOPE)
      set(_BOOST_MINOR_VERSION ${Boost_MINOR_VERSION} PARENT_SCOPE)
    endif()
  endfunction()

  find_boost_system()

  if(_BOOST_FOUND AND _BOOST_MAJOR_VERSION EQUAL 1 AND _BOOST_MINOR_VERSION LESS 69)
    LIST(APPEND Boost_COMPONENTS system)
  endif()

  set(_BOOST_FOUND)
  set(_BOOST_MAJOR_VERSION)
  set(_BOOST_MINOR_VERSION)

  FIND_PACKAGE(Boost 1.66
    COMPONENTS
      ${Boost_COMPONENTS}
    REQUIRED
  )

  # Use IMPORTED targets if defined (map to Wt variables)
  FOREACH(_target ${Boost_COMPONENTS})
    IF(TARGET Boost::${_target})
      STRING(TOUPPER ${_target} _TARGET)
      SET(Boost_${_TARGET}_LIBRARY Boost::${_target})
    ENDIF()
  ENDFOREACH()
  UNSET(_target)
  UNSET(_TARGET)

  IF (Boost_FOUND)
    IF(Boost_VERSION_MAJOR AND NOT Boost_MAJOR_VERSION)
      set(Boost_MAJOR_VERSION ${Boost_VERSION_MAJOR})
    ENDIF()
    IF(Boost_VERSION_MINOR AND NOT Boost_MINOR_VERSION)
      set(Boost_MINOR_VERSION ${Boost_MINOR_VERSION})
    ENDIF()

    # Translate variables from official cmake names to internally used names
    IF(NOT DEFINED BOOST_INCLUDE_DIRS AND DEFINED Boost_INCLUDE_DIRS)
      SET(BOOST_INCLUDE_DIRS ${Boost_INCLUDE_DIRS})
    ENDIF()
    IF(NOT DEFINED BOOST_LIB_DIRS AND DEFINED Boost_LIBRARY_DIRS)
      SET(BOOST_LIB_DIRS ${Boost_LIBRARY_DIRS})
    ENDIF()

    # If find_package actually set variables for individual libs, map them:
    if(DEFINED Boost_FILESYSTEM_LIBRARY)
      SET(BOOST_FS_LIB ${Boost_FILESYSTEM_LIBRARY})
    endif()
    if(DEFINED Boost_PROGRAM_OPTIONS_LIBRARY)
      SET(BOOST_PO_LIB ${Boost_PROGRAM_OPTIONS_LIBRARY})
    endif()
    if(DEFINED Boost_SYSTEM_LIBRARY)
      SET(BOOST_SYSTEM_LIB ${Boost_SYSTEM_LIBRARY})
    endif()
    if(DEFINED Boost_THREAD_LIBRARY)
      SET(BOOST_THREAD_LIB ${Boost_THREAD_LIBRARY})
    endif()
    IF(WIN32 AND BOOST_DYNAMIC AND BUILD_TESTS)
      SET(BOOST_UT_LIB ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
    ELSE()
      IF(NOT DEFINED BOOST_UT_LIB)
        SET(BOOST_UT_LIB "")
      ENDIF()
    ENDIF()

    SET(BOOST_WT_MT_FOUND TRUE)
    SET(BOOST_WT_FOUND TRUE)
    SET(BOOST_WTHTTP_MT_FOUND TRUE)
    SET(BOOST_WTHTTP_FOUND TRUE)

    IF(MSVC AND Boost_MAJOR_VERSION EQUAL 1 AND Boost_MINOR_VERSION LESS 70)
      # use autolink
      SET(BOOST_WT_LIBRARIES "")
      SET(BOOST_WTHTTP_LIBRARIES "")
      SET(BOOST_TEST_LIBRARIES "")
    ELSE()
      # prefer explicit variables if available, otherwise rely on the Boost::<lib> targets we set above
      SET(BOOST_WT_LIBRARIES
          ${Boost_THREAD_LIBRARY}
          ${Boost_SYSTEM_LIBRARY}
          ${Boost_FILESYSTEM_LIBRARY})
      SET(BOOST_WTHTTP_LIBRARIES
          ${Boost_THREAD_LIBRARY}
          ${Boost_PROGRAM_OPTIONS_LIBRARY}
          ${Boost_SYSTEM_LIBRARY}
          ${Boost_FILESYSTEM_LIBRARY})
      SET(BOOST_TEST_LIBRARIES ${BOOST_UT_LIB})
    ENDIF()

    IF(TARGET Boost::headers)
      SET(BOOST_WT_LIBRARIES ${BOOST_WT_LIBRARIES} Boost::headers)
      SET(BOOST_WTHTTP_LIBRARIES ${BOOST_WTHTTP_LIBRARIES} Boost::headers)
    ENDIF()
  ENDIF()
endif()

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ PROGRESS TWO @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@


# WtFindBoost.cmake - patched: in-tree Boost detection + filesystem-first include discovery
#
# Behavior:
#  - If Boost::xxx targets already exist (from add_subdirectory(boost)), map them into
#    the BOOST_* variables Wt expects and skip calling find_package()
#  - Additionally, perform a filesystem-first scan for Boost include dirs for modules
#  listed in Boost_COMPONENTS (and BOOST_INCLUDE_LIBRARIES if present).
#  - Ignores generator expressions from target properties and only appends absolute,
#    existing directories to BOOST_INCLUDE_DIRS.
#  - Otherwise fall back to the original find_package(Boost ...) logic.
#
# Based on original WtFindBoost.txt (user-provided). See embedded diagnostics when needed.
#

# default state
SET(BOOST_WT_FOUND FALSE)
SET(BOOST_WT_MT_FOUND FALSE)
SET(BOOST_WTHTTP_FOUND FALSE)
SET(BOOST_WTHTTP_MT_FOUND FALSE)
SET(BOOST_SUPPORT_LIBRARIES "")

IF(NOT DEFINED Boost_ADDITIONAL_VERSIONS)
  SET(Boost_ADDITIONAL_VERSIONS
    "1.66" "1.67" "1.68" "1.69" "1.70" "1.71" "1.72" "1.73" "1.74" "1.75"
    "1.76" "1.77" "1.78" "1.79" "1.80" "1.81" "1.82" "1.83" "1.84" "1.85"
    "1.86" "1.87" "1.88")
ENDIF()

SET(Boost_USE_MULTITHREADED ON)

IF(DEFINED BOOST_PREFIX AND NOT DEFINED BOOST_ROOT)
  SET(BOOST_ROOT ${BOOST_PREFIX})
ENDIF()

IF(WIN32)
  IF(BOOST_DYNAMIC)
    SET(Boost_USE_STATIC_LIBS OFF)
  ELSE()
    SET(Boost_USE_STATIC_LIBS ON)
  ENDIF()
ENDIF()

SET(Boost_DEBUG ON)

# Default components Wt needs (adjust as required)
SET(Boost_COMPONENTS
    program_options
    thread
    asio
    filesystem
    chrono
    system
    numeric
)

IF(WT_CPP17_FILESYSTEM_IMPLEMENTATION STREQUAL "boost")
  LIST(APPEND Boost_COMPONENTS filesystem)
ENDIF()

IF(WIN32 AND BOOST_DYNAMIC AND BUILD_TESTS)
  LIST(APPEND Boost_COMPONENTS unit_test_framework)
ENDIF()

#
# --------- NEW: Detect an in-tree Boost (targets created by add_subdirectory) ---------
#
set(_wt_in_tree_boost FALSE)
set(_wt_boost_interface_includes "")  # keep generator-expr includes if we need them (we won't rely on them)
set(_wt_boost_collected_libs "")

foreach(_comp ${Boost_COMPONENTS})
  if(TARGET Boost::${_comp})
    set(_wt_in_tree_boost TRUE)
    list(APPEND _wt_boost_collected_libs Boost::${_comp})
    # collect INTERFACE_INCLUDE_DIRECTORIES but do not rely on generator expressions:
    get_target_property(_incdirs Boost::${_comp} INTERFACE_INCLUDE_DIRECTORIES)
    if(_incdirs)
      list(APPEND _wt_boost_interface_includes ${_incdirs})
    endif()
  endif()
endforeach()

if(_wt_in_tree_boost)
  message(STATUS "WtFindBoost: Boost:: targets detected (in-tree Boost). Using filesystem-first include discovery.")

  set(Boost_FOUND TRUE CACHE INTERNAL "Found in-tree Boost")
  set(BOOST_LIB_DIRS "")

  # Map commonly-used Boost::<lib> targets into Wt variables if present
  set(_wt_libs "")
  foreach(_lib program_options thread system filesystem asio chrono numeric)
    if(TARGET Boost::${_lib})
      list(APPEND _wt_libs Boost::${_lib})
      string(TOUPPER ${_lib} _ULIB)
      set(BOOST_${_ULIB}_LIB Boost::${_lib})
    endif()
  endforeach()

  set(BOOST_WT_LIBRARIES ${_wt_libs})
  set(BOOST_WTHTTP_LIBRARIES ${_wt_libs})

  if(TARGET Boost::headers)
    list(APPEND BOOST_WT_LIBRARIES Boost::headers)
    list(APPEND BOOST_WTHTTP_LIBRARIES Boost::headers)
  endif()

  set(BOOST_WT_FOUND TRUE)
  set(BOOST_WT_MT_FOUND TRUE)
  set(BOOST_WTHTTP_FOUND TRUE)
  set(BOOST_WTHTTP_MT_FOUND TRUE)

  #
  # --------- Filesystem-first include discovery (component-limited) ----------
  #
  # Goal: for each component in Boost_COMPONENTS and any in BOOST_INCLUDE_LIBRARIES,
  # collect absolute include directories that actually exist on disk:
  #   - <module>/include
  #   - <module>/*/include
  #   - <module>/<child>   (if child contains headers directly)
  #   - header-parent dirs for header files found under module
  #

  set(_collected_includes_raw "")

  # Modules to scan: union of Boost_COMPONENTS and BOOST_INCLUDE_LIBRARIES
  set(_mods "")
  if(DEFINED BOOST_INCLUDE_LIBRARIES)
    list(APPEND _mods ${BOOST_INCLUDE_LIBRARIES})
  endif()
  list(APPEND _mods ${Boost_COMPONENTS})
  list(REMOVE_DUPLICATES _mods)

  # ---------------------------------------------------------------------------
  # Improved helper function: collect_module_includes
  # - Accepts module include layouts
  # - Keeps only directories that are 'include' dirs or contain headers (direct)
  # - Filters out noisy paths (docs, tests, examples, .github, etc.)
  function(collect_module_includes _moddir _out_var)
    if(NOT _moddir)
      set(${_out_var} "" PARENT_SCOPE)
      return()
    endif()

    set(_raw_list "")

    # 1) module-level include folder
    if(EXISTS "${_moddir}/include" AND IS_DIRECTORY "${_moddir}/include")
      list(APPEND _raw_list "${_moddir}/include")
    endif()

    # 2) find any nested .../include directories (recursive)
    file(GLOB_RECURSE _found_inc_dirs LIST_DIRECTORIES true "${_moddir}/**/include")
    foreach(_d IN LISTS _found_inc_dirs)
      if(IS_DIRECTORY "${_d}")
        list(APPEND _raw_list "${_d}")
      endif()
    endforeach()

    # 3) collect header-parent dirs for headers found under module (recursive)
    file(GLOB_RECURSE _found_headers
         "${_moddir}/**/*.hpp"
         "${_moddir}/**/*.h"
         "${_moddir}/**/*.ipp"
         "${_moddir}/**/*.inl")
    foreach(_hdr IN LISTS _found_headers)
      get_filename_component(_hdr_dir "${_hdr}" DIRECTORY)
      if(_hdr_dir AND IS_DIRECTORY "${_hdr_dir}")
        list(APPEND _raw_list "${_hdr_dir}")
      endif()
    endforeach()

    # 4) immediate child folders of _moddir that contain headers (non-recursive)
    file(GLOB _children LIST_DIRECTORIES true "${_moddir}/*")
    foreach(_child IN LISTS _children)
      if(IS_DIRECTORY "${_child}")
        # include child/include if present
        if(EXISTS "${_child}/include" AND IS_DIRECTORY "${_child}/include")
          list(APPEND _raw_list "${_child}/include")
        endif()
        # add child if it directly contains headers (non-recursive)
        file(GLOB _child_hdrs "${_child}/*.{hpp,h,ipp,inl}")
        if(_child_hdrs)
          list(APPEND _raw_list "${_child}")
        endif()
      endif()
    endforeach()

    # dedupe
    list(REMOVE_DUPLICATES _raw_list)

    # noise filter regex (skip these common non-include folders)
    # anchored to any path segment; common noise folders are skipped
    set(_noise_regex "/(doc|docs|example|examples|test|tests|bench|benchmark|benchmarks|\.github|cmake_subdir_test|\.git)/")

    set(_final_list "")
    foreach(_p IN LISTS _raw_list)
      if(NOT _p)
        continue()
      endif()
      # skip generator expressions or weird tokens
      if("${_p}" MATCHES "\$<")
        continue()
      endif()

      # normalize
      get_filename_component(_abs_p "${_p}" ABSOLUTE)

      # quick noise exclusion by path pattern
      if("${_abs_p}" MATCHES "${_noise_regex}")
        continue()
      endif()

      # Accept if directory name is 'include'
      get_filename_component(_basename "${_abs_p}" NAME)
      if(_basename STREQUAL "include")
        if(IS_DIRECTORY "${_abs_p}")
          list(APPEND _final_list "${_abs_p}")
          continue()
        endif()
      endif()

      # Accept if directory contains header files directly (non recursive)
      file(GLOB _hdrs_direct "${_abs_p}/*.{hpp,h,ipp,inl}")
      if(_hdrs_direct)
        list(APPEND _final_list "${_abs_p}")
        continue()
      endif()

      # Accept if immediate children contain header files (one level deep)
      file(GLOB _immediate_children LIST_DIRECTORIES true "${_abs_p}/*")
      set(_accepted FALSE)
      foreach(_ch IN LISTS _immediate_children)
        if(IS_DIRECTORY "${_ch}")
          file(GLOB _child_hdrs "${_ch}/*.{hpp,h,ipp,inl}")
          if(_child_hdrs)
            set(_accepted TRUE)
            break()
          endif()
        endif()
      endforeach()
      if(_accepted)
        list(APPEND _final_list "${_abs_p}")
        continue()
      endif()

      # otherwise skip
    endforeach()

    # final dedupe + sanity filter (ensure exists) and resolve REALPATH
    list(REMOVE_DUPLICATES _final_list)
    set(_out_list "")
    foreach(_q IN LISTS _final_list)
      if(IS_DIRECTORY "${_q}")
        get_filename_component(_realq "${_q}" REALPATH)
        if(_realq AND IS_DIRECTORY "${_realq}")
          list(APPEND _out_list "${_realq}")
        endif()
      endif()
    endforeach()

    list(REMOVE_DUPLICATES _out_list)
    set(${_out_var} "${_out_list}" PARENT_SCOPE)
  endfunction()
  # ---------------------------------------------------------------------------

  foreach(_mod IN LISTS _mods)
    string(STRIP "${_mod}" _mod_trim)
    if(NOT _mod_trim)
      continue()
    endif()

    # Prefer BOOST_ROOT or fall back to project source dirs
    if(DEFINED BOOST_ROOT)
      set(_moddir "${BOOST_ROOT}/libs/${_mod_trim}")
    else()
      set(_moddir "${CMAKE_SOURCE_DIR}/boost/libs/${_mod_trim}")
    endif()

    # If moddir missing, also try current source dir path (some projects configure from subdir)
    if(NOT EXISTS "${_moddir}")
      set(_moddir_alt "${CMAKE_CURRENT_SOURCE_DIR}/boost/libs/${_mod_trim}")
      if(EXISTS "${_moddir_alt}")
        set(_moddir "${_moddir_alt}")
      endif()
    endif()

    if(NOT EXISTS "${_moddir}")
      # module not present in source tree - skip
      message(STATUS "WtFindBoost: module not present, skipping: ${_mod_trim}")
      continue()
    endif()

    collect_module_includes("${_moddir}" _module_incs)
    foreach(_inc IN LISTS _module_incs)
      list(APPEND _collected_includes_raw "${_inc}")
    endforeach()

  endforeach() # _mods

  # Always append top-level boost source dir as fallback (use BOOST_ROOT if set)
  if(DEFINED BOOST_ROOT)
    list(APPEND _collected_includes_raw "${BOOST_ROOT}")
  else()
    list(APPEND _collected_includes_raw "${CMAKE_SOURCE_DIR}/boost")
  endif()

  # Deduplicate collected raw candidates
  list(REMOVE_DUPLICATES _collected_includes_raw)

  # Final pass: run collect_module_includes on any module-root-like paths to ensure sanity
  set(_kept "")
  foreach(_p IN LISTS _collected_includes_raw)
    if(NOT _p)
      continue()
    endif()

    # skip generator-expressions
    if("${_p}" MATCHES "\$<.*:.*>")
      continue()
    endif()

    # If this path is a module root (e.g. .../boost/libs/<mod>) then run helper on it
    if(IS_DIRECTORY "${_p}")
      collect_module_includes("${_p}" _tmpincs)
      foreach(_i IN LISTS _tmpincs)
        list(APPEND _kept "${_i}")
      endforeach()
    endif()
  endforeach()

  list(REMOVE_DUPLICATES _kept)

  # Export as BOOST_INCLUDE_DIRS (CACHE INTERNAL to avoid accidental override)
  set(BOOST_INCLUDE_DIRS ${_kept} CACHE INTERNAL "Collected in-tree Boost include dirs (filtered)")

  # Add to global include path for the configure/build process.
  # Ensure BOOST_INCLUDE_DIRS contains absolute, deduplicated paths
  if(_kept)
    set(_abs_includes "")
    foreach(_inc IN LISTS _kept)
      get_filename_component(_abs "${_inc}" REALPATH)
      if(_abs AND IS_DIRECTORY "${_abs}")
        list(APPEND _abs_includes "${_abs}")
      endif()
    endforeach()
    list(REMOVE_DUPLICATES _abs_includes)
    set(BOOST_INCLUDE_DIRS ${_abs_includes} CACHE INTERNAL "Collected in-tree Boost include dirs (filtered)")

    # Append include dirs to the directory properties so they appear in generated build rules
    # Append to top-level source dir and current source dir to cover different configure scopes
    if(BOOST_INCLUDE_DIRS)
      set_property(DIRECTORY ${CMAKE_SOURCE_DIR} APPEND PROPERTY INCLUDE_DIRECTORIES ${BOOST_INCLUDE_DIRS})
      if(NOT CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
        set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} APPEND PROPERTY INCLUDE_DIRECTORIES ${BOOST_INCLUDE_DIRS})
      endif()

      # Also try to attach include dirs to any discovered Boost:: targets (if safe)
      foreach(_t IN LISTS BOOST_WT_LIBRARIES)
      if(TARGET ${_t})
        # If the target is an ALIAS, the ALIASED_TARGET property will be set.
        # Use that to avoid calling target_include_directories on ALIAS targets.
        get_target_property(_aliased ${_t} ALIASED_TARGET)
        if(DEFINED _aliased AND NOT "${_aliased}" STREQUAL "NOTFOUND")
          message(STATUS "WtFindBoost: skipping target_include_directories for ALIAS target ${_t} (aliases ${_aliased})")
        else()
          # Safe: not an alias. Add the include dirs as INTERFACE so consumers of the Boost target get them.
          # Guard again: only call if target supports usage requirements (ignore if TYPE empty)
          get_target_property(_t_type ${_t} TYPE)
          if(NOT "${_t_type}" STREQUAL "UNKNOWN")
            message(VERBOSE "WtFindBoost: attaching BOOST_INCLUDE_DIRS to target ${_t}")
            target_include_directories(${_t} INTERFACE ${BOOST_INCLUDE_DIRS})
          else()
            message(STATUS "WtFindBoost: not attaching include dirs to target ${_t} (unknown type)")
          endif()
        endif()
      endif()
    endforeach()

      foreach(_inc IN LISTS BOOST_INCLUDE_DIRS)
        message(STATUS "WtFindBoost: adding include dir => ${_inc}")
      endforeach()
    endif()
  else()
    message(STATUS "WtFindBoost: no filtered include dirs to add")
    # If nothing found, as a diagnostic step, also output interface include props (generator-expr) if any
    if(_wt_boost_interface_includes)
      message(STATUS "  Note: Boost targets expose INTERFACE include properties (generator-expr); showing them for debugging:")
      foreach(_gi IN LISTS _wt_boost_interface_includes)
        message(STATUS "    ${_gi}")
      endforeach()
    endif()
  endif()

  # Debug summary: report what we discovered originally (helpful for verification)
  list(LENGTH _collected_includes_raw _len)
  message(STATUS "WtFindBoost: discovered ${_len} physical include directories under boost/libs (pre-filter, showing up to 50):")
  if(_len GREATER 0)
    if(_len LESS 50)
      set(_max_show ${_len})
    else()
      set(_max_show 50)
    endif()
    foreach(_i RANGE 0 ${_max_show})
      if(_i LESS _len)
        list(GET _collected_includes_raw ${_i} _show)
        message(STATUS "  ${_show}")
      endif()
    endforeach()
  endif()

  # Done: set other expected values (some were set earlier)
  set(BOOST_WT_FOUND TRUE)
  set(BOOST_WT_MT_FOUND TRUE)
  set(BOOST_WTHTTP_FOUND TRUE)
  set(BOOST_WTHTTP_MT_FOUND TRUE)

  # Cleanup temporaries
  unset(_collected_includes_raw)
  unset(_found_inc_dirs)
  unset(_found_headers)
  unset(_child_dirs)
  unset(_child_hdrs)
  unset(_final_includes)
  unset(_mods)
  unset(_wt_boost_interface_includes)
  unset(_wt_boost_collected_libs)
  unset(_wt_in_tree_boost)
  unset(_wt_libs)

#
# --------- END in-tree detection and filesystem discovery -------------------
#
else()
  #
  # --- No in-tree targets — fall back to CMake's find_package(Boost) ---
  #
  function(find_boost_system)
    find_package(Boost 1.66 QUIET
      COMPONENTS
        system
    )

    set(_BOOST_FOUND ${Boost_FOUND} PARENT_SCOPE)
    if(Boost_VERSION_MAJOR)
      set(_BOOST_MAJOR_VERSION ${Boost_VERSION_MAJOR} PARENT_SCOPE)
      set(_BOOST_MINOR_VERSION ${Boost_VERSION_MINOR} PARENT_SCOPE)
    else()
      set(_BOOST_MAJOR_VERSION ${Boost_MAJOR_VERSION} PARENT_SCOPE)
      set(_BOOST_MINOR_VERSION ${Boost_MINOR_VERSION} PARENT_SCOPE)
    endif()
  endfunction()

  find_boost_system()

  if(_BOOST_FOUND AND _BOOST_MAJOR_VERSION EQUAL 1 AND _BOOST_MINOR_VERSION LESS 69)
    LIST(APPEND Boost_COMPONENTS system)
  endif()

  set(_BOOST_FOUND)
  set(_BOOST_MAJOR_VERSION)
  set(_BOOST_MINOR_VERSION)

  FIND_PACKAGE(Boost 1.66
    COMPONENTS
      ${Boost_COMPONENTS}
    REQUIRED
  )

  # Use IMPORTED targets if defined (map to Wt variables)
  FOREACH(_target ${Boost_COMPONENTS})
    IF(TARGET Boost::${_target})
      STRING(TOUPPER ${_target} _TARGET)
      SET(Boost_${_TARGET}_LIBRARY Boost::${_target})
    ENDIF()
  ENDFOREACH()
  UNSET(_target)
  UNSET(_TARGET)

  IF (Boost_FOUND)
    IF(Boost_VERSION_MAJOR AND NOT Boost_MAJOR_VERSION)
      set(Boost_MAJOR_VERSION ${Boost_VERSION_MAJOR})
    ENDIF()
    IF(Boost_VERSION_MINOR AND NOT Boost_MINOR_VERSION)
      set(Boost_MINOR_VERSION ${Boost_MINOR_VERSION})
    ENDIF()

    # Translate variables from official cmake names to internally used names
    IF(NOT DEFINED BOOST_INCLUDE_DIRS AND DEFINED Boost_INCLUDE_DIRS)
      SET(BOOST_INCLUDE_DIRS ${Boost_INCLUDE_DIRS})
    ENDIF()
    IF(NOT DEFINED BOOST_LIB_DIRS AND DEFINED Boost_LIBRARY_DIRS)
      SET(BOOST_LIB_DIRS ${Boost_LIBRARY_DIRS})
    ENDIF()

    # If find_package actually set variables for individual libs, map them:
    if(DEFINED Boost_FILESYSTEM_LIBRARY)
      SET(BOOST_FS_LIB ${Boost_FILESYSTEM_LIBRARY})
    endif()
    if(DEFINED Boost_PROGRAM_OPTIONS_LIBRARY)
      SET(BOOST_PO_LIB ${Boost_PROGRAM_OPTIONS_LIBRARY})
    endif()
    if(DEFINED Boost_SYSTEM_LIBRARY)
      SET(BOOST_SYSTEM_LIB ${Boost_SYSTEM_LIBRARY})
    endif()
    if(DEFINED Boost_THREAD_LIBRARY)
      SET(BOOST_THREAD_LIB ${Boost_THREAD_LIBRARY})
    endif()
    IF(WIN32 AND BOOST_DYNAMIC AND BUILD_TESTS)
      SET(BOOST_UT_LIB ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
    ELSE()
      IF(NOT DEFINED BOOST_UT_LIB)
        SET(BOOST_UT_LIB "")
      ENDIF()
    ENDIF()

    SET(BOOST_WT_MT_FOUND TRUE)
    SET(BOOST_WT_FOUND TRUE)
    SET(BOOST_WTHTTP_MT_FOUND TRUE)
    SET(BOOST_WTHTTP_FOUND TRUE)

    IF(MSVC AND Boost_MAJOR_VERSION EQUAL 1 AND Boost_MINOR_VERSION LESS 70)
      # use autolink
      SET(BOOST_WT_LIBRARIES "")
      SET(BOOST_WTHTTP_LIBRARIES "")
      SET(BOOST_TEST_LIBRARIES "")
    ELSE()
      # prefer explicit variables if available, otherwise rely on the Boost::<lib> targets we set above
      SET(BOOST_WT_LIBRARIES
          ${Boost_THREAD_LIBRARY}
          ${Boost_SYSTEM_LIBRARY}
          ${Boost_FILESYSTEM_LIBRARY})
      SET(BOOST_WTHTTP_LIBRARIES
          ${Boost_THREAD_LIBRARY}
          ${Boost_PROGRAM_OPTIONS_LIBRARY}
          ${Boost_SYSTEM_LIBRARY}
          ${Boost_FILESYSTEM_LIBRARY})
      SET(BOOST_TEST_LIBRARIES ${BOOST_UT_LIB})
    ENDIF()

    IF(TARGET Boost::headers)
      SET(BOOST_WT_LIBRARIES ${BOOST_WT_LIBRARIES} Boost::headers)
      SET(BOOST_WTHTTP_LIBRARIES ${BOOST_WTHTTP_LIBRARIES} Boost::headers)
    ENDIF()
  ENDIF()
endif()


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ PROGRESS THREE @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@


# WtFindBoost.cmake - patched: in-tree Boost detection + filesystem-first include discovery
#
# Behavior:
#  - If Boost::xxx targets already exist (from add_subdirectory(boost)), map them into
#    the BOOST_* variables Wt expects and skip calling find_package()
#  - Additionally, perform a filesystem-first scan for Boost include dirs for modules
#  listed in Boost_COMPONENTS (and BOOST_INCLUDE_LIBRARIES if present).
#  - Ignores generator expressions from target properties and only appends absolute,
#    existing directories to BOOST_INCLUDE_DIRS.
#  - Otherwise fall back to the original find_package(Boost ...) logic.
#
# Based on original WtFindBoost.txt (user-provided). See embedded diagnostics when needed.
#

# default state
SET(BOOST_WT_FOUND FALSE)
SET(BOOST_WT_MT_FOUND FALSE)
SET(BOOST_WTHTTP_FOUND FALSE)
SET(BOOST_WTHTTP_MT_FOUND FALSE)
SET(BOOST_SUPPORT_LIBRARIES "")

IF(NOT DEFINED Boost_ADDITIONAL_VERSIONS)
  SET(Boost_ADDITIONAL_VERSIONS
    "1.66" "1.67" "1.68" "1.69" "1.70" "1.71" "1.72" "1.73" "1.74" "1.75"
    "1.76" "1.77" "1.78" "1.79" "1.80" "1.81" "1.82" "1.83" "1.84" "1.85"
    "1.86" "1.87" "1.88")
ENDIF()

SET(Boost_USE_MULTITHREADED ON)

IF(DEFINED BOOST_PREFIX AND NOT DEFINED BOOST_ROOT)
  SET(BOOST_ROOT ${BOOST_PREFIX})
ENDIF()

IF(WIN32)
  IF(BOOST_DYNAMIC)
    SET(Boost_USE_STATIC_LIBS OFF)
  ELSE()
    SET(Boost_USE_STATIC_LIBS ON)
  ENDIF()
ENDIF()

SET(Boost_DEBUG ON)

# Default components Wt needs (adjust as required)
SET(Boost_COMPONENTS
    program_options
    thread
    asio
    filesystem
    chrono
    system
    numeric
)

IF(WT_CPP17_FILESYSTEM_IMPLEMENTATION STREQUAL "boost")
  LIST(APPEND Boost_COMPONENTS filesystem)
ENDIF()

IF(WIN32 AND BOOST_DYNAMIC AND BUILD_TESTS)
  LIST(APPEND Boost_COMPONENTS unit_test_framework)
ENDIF()

#
# --------- NEW: Detect an in-tree Boost (targets created by add_subdirectory) ---------
#
set(_wt_in_tree_boost FALSE)
set(_wt_boost_interface_includes "")  # keep generator-expr includes if we need them (we won't rely on them)
set(_wt_boost_collected_libs "")

foreach(_comp ${Boost_COMPONENTS})
  if(TARGET Boost::${_comp})
    set(_wt_in_tree_boost TRUE)
    list(APPEND _wt_boost_collected_libs Boost::${_comp})
    # collect INTERFACE_INCLUDE_DIRECTORIES but do not rely on generator expressions:
    get_target_property(_incdirs Boost::${_comp} INTERFACE_INCLUDE_DIRECTORIES)
    if(_incdirs)
      list(APPEND _wt_boost_interface_includes ${_incdirs})
    endif()
  endif()
endforeach()

if(_wt_in_tree_boost)
  message(STATUS "WtFindBoost: Boost:: targets detected (in-tree Boost). Using filesystem-first include discovery.")

  set(Boost_FOUND TRUE CACHE INTERNAL "Found in-tree Boost")
  set(BOOST_LIB_DIRS "")

  # Map commonly-used Boost::<lib> targets into Wt variables if present
  set(_wt_libs "")
  foreach(_lib program_options thread system filesystem asio chrono numeric)
    if(TARGET Boost::${_lib})
      list(APPEND _wt_libs Boost::${_lib})
      string(TOUPPER ${_lib} _ULIB)
      set(BOOST_${_ULIB}_LIB Boost::${_lib})
    endif()
  endforeach()

  set(BOOST_WT_LIBRARIES ${_wt_libs})
  set(BOOST_WTHTTP_LIBRARIES ${_wt_libs})

  if(TARGET Boost::headers)
    list(APPEND BOOST_WT_LIBRARIES Boost::headers)
    list(APPEND BOOST_WTHTTP_LIBRARIES Boost::headers)
  endif()

  set(BOOST_WT_FOUND TRUE)
  set(BOOST_WT_MT_FOUND TRUE)
  set(BOOST_WTHTTP_FOUND TRUE)
  set(BOOST_WTHTTP_MT_FOUND TRUE)

  #
  # --------- Filesystem-first include discovery (component-limited) ----------
  #
  # Goal: for each component in Boost_COMPONENTS and any in BOOST_INCLUDE_LIBRARIES,
  # collect absolute include directories that actually exist on disk:
  #   - <module>/include
  #   - <module>/*/include
  #   - <module>/<child>   (if child contains headers directly)
  #   - header-parent dirs for header files found under module
  #

  set(_collected_includes_raw "")

  # Modules to scan: union of Boost_COMPONENTS and BOOST_INCLUDE_LIBRARIES
  set(_mods "")
  if(DEFINED BOOST_INCLUDE_LIBRARIES)
    list(APPEND _mods ${BOOST_INCLUDE_LIBRARIES})
  endif()
  list(APPEND _mods ${Boost_COMPONENTS})
  list(REMOVE_DUPLICATES _mods)

  # ---------------------------------------------------------------------------
  # Improved helper function: collect_module_includes
  # - Accepts module include layouts
  # - Keeps only directories that are 'include' dirs or contain headers (direct)
  # - Filters out noisy paths (docs, tests, examples, .github, etc.)
  function(collect_module_includes _moddir _out_var)
    if(NOT _moddir)
      set(${_out_var} "" PARENT_SCOPE)
      return()
    endif()

    set(_raw_list "")

    # 1) module-level include folder
    if(EXISTS "${_moddir}/include" AND IS_DIRECTORY "${_moddir}/include")
      list(APPEND _raw_list "${_moddir}/include")
    endif()

    # 2) find any nested .../include directories (recursive)
    file(GLOB_RECURSE _found_inc_dirs LIST_DIRECTORIES true "${_moddir}/**/include")
    foreach(_d IN LISTS _found_inc_dirs)
      if(IS_DIRECTORY "${_d}")
        list(APPEND _raw_list "${_d}")
      endif()
    endforeach()

    # 3) collect header-parent dirs for headers found under module (recursive)
    file(GLOB_RECURSE _found_headers
         "${_moddir}/**/*.hpp"
         "${_moddir}/**/*.h"
         "${_moddir}/**/*.ipp"
         "${_moddir}/**/*.inl")
    foreach(_hdr IN LISTS _found_headers)
      get_filename_component(_hdr_dir "${_hdr}" DIRECTORY)
      if(_hdr_dir AND IS_DIRECTORY "${_hdr_dir}")
        list(APPEND _raw_list "${_hdr_dir}")
      endif()
    endforeach()

    # 4) immediate child folders of _moddir that contain headers (non-recursive)
    file(GLOB _children LIST_DIRECTORIES true "${_moddir}/*")
    foreach(_child IN LISTS _children)
      if(IS_DIRECTORY "${_child}")
        # include child/include if present
        if(EXISTS "${_child}/include" AND IS_DIRECTORY "${_child}/include")
          list(APPEND _raw_list "${_child}/include")
        endif()
        # add child if it directly contains headers (non-recursive)
        file(GLOB _child_hdrs "${_child}/*.{hpp,h,ipp,inl}")
        if(_child_hdrs)
          list(APPEND _raw_list "${_child}")
        endif()
      endif()
    endforeach()

    # dedupe
    list(REMOVE_DUPLICATES _raw_list)

    # noise filter regex (skip these common non-include folders)
    # anchored to any path segment; common noise folders are skipped
    set(_noise_regex "/(doc|docs|example|examples|test|tests|bench|benchmark|benchmarks|\.github|cmake_subdir_test|\.git)/")

    set(_final_list "")
    foreach(_p IN LISTS _raw_list)
      if(NOT _p)
        continue()
      endif()
      # skip generator expressions or weird tokens
      if("${_p}" MATCHES "\$<")
        continue()
      endif()

      # normalize
      get_filename_component(_abs_p "${_p}" ABSOLUTE)

      # quick noise exclusion by path pattern
      if("${_abs_p}" MATCHES "${_noise_regex}")
        continue()
      endif()

      # Accept if directory name is 'include'
      get_filename_component(_basename "${_abs_p}" NAME)
      if(_basename STREQUAL "include")
        if(IS_DIRECTORY "${_abs_p}")
          list(APPEND _final_list "${_abs_p}")
          continue()
        endif()
      endif()

      # Accept if directory contains header files directly (non recursive)
      file(GLOB _hdrs_direct "${_abs_p}/*.{hpp,h,ipp,inl}")
      if(_hdrs_direct)
        list(APPEND _final_list "${_abs_p}")
        continue()
      endif()

      # Accept if immediate children contain header files (one level deep)
      file(GLOB _immediate_children LIST_DIRECTORIES true "${_abs_p}/*")
      set(_accepted FALSE)
      foreach(_ch IN LISTS _immediate_children)
        if(IS_DIRECTORY "${_ch}")
          file(GLOB _child_hdrs "${_ch}/*.{hpp,h,ipp,inl}")
          if(_child_hdrs)
            set(_accepted TRUE)
            break()
          endif()
        endif()
      endforeach()
      if(_accepted)
        list(APPEND _final_list "${_abs_p}")
        continue()
      endif()

      # otherwise skip
    endforeach()

    # final dedupe + sanity filter (ensure exists) and resolve REALPATH
    list(REMOVE_DUPLICATES _final_list)
    set(_out_list "")
    foreach(_q IN LISTS _final_list)
      if(IS_DIRECTORY "${_q}")
        get_filename_component(_realq "${_q}" REALPATH)
        if(_realq AND IS_DIRECTORY "${_realq}")
          list(APPEND _out_list "${_realq}")
        endif()
      endif()
    endforeach()

    list(REMOVE_DUPLICATES _out_list)
    set(${_out_var} "${_out_list}" PARENT_SCOPE)
  endfunction()
  # ---------------------------------------------------------------------------

  foreach(_mod IN LISTS _mods)
    string(STRIP "${_mod}" _mod_trim)
    if(NOT _mod_trim)
      continue()
    endif()

    # Prefer BOOST_ROOT or fall back to project source dirs
    if(DEFINED BOOST_ROOT)
      set(_moddir "${BOOST_ROOT}/libs/${_mod_trim}")
    else()
      set(_moddir "${CMAKE_SOURCE_DIR}/boost/libs/${_mod_trim}")
    endif()

    # If moddir missing, also try current source dir path (some projects configure from subdir)
    if(NOT EXISTS "${_moddir}")
      set(_moddir_alt "${CMAKE_CURRENT_SOURCE_DIR}/boost/libs/${_mod_trim}")
      if(EXISTS "${_moddir_alt}")
        set(_moddir "${_moddir_alt}")
      endif()
    endif()

    if(NOT EXISTS "${_moddir}")
      # module not present in source tree - skip
      message(STATUS "WtFindBoost: module not present, skipping: ${_mod_trim}")
      continue()
    endif()

    collect_module_includes("${_moddir}" _module_incs)
    foreach(_inc IN LISTS _module_incs)
      list(APPEND _collected_includes_raw "${_inc}")
    endforeach()

  endforeach() # _mods

  # Always append top-level boost source dir as fallback (use BOOST_ROOT if set)
  if(DEFINED BOOST_ROOT)
    list(APPEND _collected_includes_raw "${BOOST_ROOT}")
  else()
    list(APPEND _collected_includes_raw "${CMAKE_SOURCE_DIR}/boost")
  endif()

  # Deduplicate collected raw candidates
  list(REMOVE_DUPLICATES _collected_includes_raw)

  # Final pass: run collect_module_includes on any module-root-like paths to ensure sanity
  set(_kept "")
  foreach(_p IN LISTS _collected_includes_raw)
    if(NOT _p)
      continue()
    endif()

    # skip generator-expressions
    if("${_p}" MATCHES "\$<.*:.*>")
      continue()
    endif()

    # If this path is a module root (e.g. .../boost/libs/<mod>) then run helper on it
    if(IS_DIRECTORY "${_p}")
      collect_module_includes("${_p}" _tmpincs)
      foreach(_i IN LISTS _tmpincs)
        list(APPEND _kept "${_i}")
      endforeach()
    endif()
  endforeach()

  list(REMOVE_DUPLICATES _kept)

  # Export as BOOST_INCLUDE_DIRS (CACHE INTERNAL to avoid accidental override)
  set(BOOST_INCLUDE_DIRS ${_kept} CACHE INTERNAL "Collected in-tree Boost include dirs (filtered)")

  # Add to global include path for the configure/build process.
  # Ensure BOOST_INCLUDE_DIRS contains absolute, deduplicated paths
  if(_kept)
    set(_abs_includes "")
    foreach(_inc IN LISTS _kept)
      get_filename_component(_abs "${_inc}" REALPATH)
      if(_abs AND IS_DIRECTORY "${_abs}")
        list(APPEND _abs_includes "${_abs}")
      endif()
    endforeach()
    list(REMOVE_DUPLICATES _abs_includes)
    set(BOOST_INCLUDE_DIRS ${_abs_includes} CACHE INTERNAL "Collected in-tree Boost include dirs (filtered)")

    # Append include dirs to the directory properties so they appear in generated build rules
    # Append to top-level source dir and current source dir to cover different configure scopes
    if(BOOST_INCLUDE_DIRS)
      set_property(DIRECTORY ${CMAKE_SOURCE_DIR} APPEND PROPERTY INCLUDE_DIRECTORIES ${BOOST_INCLUDE_DIRS})
    # Also append to the binary directory so generated build files (Ninja/MSVC etc.) pick up includes
    set_property(DIRECTORY ${CMAKE_BINARY_DIR} APPEND PROPERTY INCLUDE_DIRECTORIES ${BOOST_INCLUDE_DIRS})
    if(NOT CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
      set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} APPEND PROPERTY INCLUDE_DIRECTORIES ${BOOST_INCLUDE_DIRS})
    endif()
    if(NOT CMAKE_CURRENT_BINARY_DIR STREQUAL CMAKE_BINARY_DIR)
      set_property(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} APPEND PROPERTY INCLUDE_DIRECTORIES ${BOOST_INCLUDE_DIRS})
    endif()

      # Also try to attach include dirs to any discovered Boost:: targets (if safe)
      foreach(_t IN LISTS BOOST_WT_LIBRARIES)
      if(TARGET ${_t})
        # If the target is an ALIAS, the ALIASED_TARGET property will be set.
        # Use that to avoid calling target_include_directories on ALIAS targets.
        get_target_property(_aliased ${_t} ALIASED_TARGET)
        if(DEFINED _aliased AND NOT "${_aliased}" STREQUAL "NOTFOUND")
          message(STATUS "WtFindBoost: skipping target_include_directories for ALIAS target ${_t} (aliases ${_aliased})")
        else()
          # Safe: not an alias. Add the include dirs as INTERFACE so consumers of the Boost target get them.
          # Guard again: only call if target supports usage requirements (ignore if TYPE empty)
          get_target_property(_t_type ${_t} TYPE)
          if(NOT "${_t_type}" STREQUAL "UNKNOWN")
            message(VERBOSE "WtFindBoost: attaching BOOST_INCLUDE_DIRS to target ${_t}")
            target_include_directories(${_t} INTERFACE ${BOOST_INCLUDE_DIRS})
          else()
            message(STATUS "WtFindBoost: not attaching include dirs to target ${_t} (unknown type)")
          endif()
        endif()
      endif()
    endforeach()

      foreach(_inc IN LISTS BOOST_INCLUDE_DIRS)
        message(STATUS "WtFindBoost: adding include dir => ${_inc}")
      endforeach()
    endif()
  else()
    message(STATUS "WtFindBoost: no filtered include dirs to add")
    # If nothing found, as a diagnostic step, also output interface include props (generator-expr) if any
    if(_wt_boost_interface_includes)
      message(STATUS "  Note: Boost targets expose INTERFACE include properties (generator-expr); showing them for debugging:")
      foreach(_gi IN LISTS _wt_boost_interface_includes)
        message(STATUS "    ${_gi}")
      endforeach()
    endif()
  endif()

  # Debug summary: report what we discovered originally (helpful for verification)
  list(LENGTH _collected_includes_raw _len)
  message(STATUS "WtFindBoost: discovered ${_len} physical include directories under boost/libs (pre-filter, showing up to 50):")
  if(_len GREATER 0)
    if(_len LESS 50)
      set(_max_show ${_len})
    else()
      set(_max_show 50)
    endif()
    foreach(_i RANGE 0 ${_max_show})
      if(_i LESS _len)
        list(GET _collected_includes_raw ${_i} _show)
        message(STATUS "  ${_show}")
      endif()
    endforeach()
  endif()

  # Done: set other expected values (some were set earlier)
  set(BOOST_WT_FOUND TRUE)
  set(BOOST_WT_MT_FOUND TRUE)
  set(BOOST_WTHTTP_FOUND TRUE)
  set(BOOST_WTHTTP_MT_FOUND TRUE)

  # Cleanup temporaries
  unset(_collected_includes_raw)
  unset(_found_inc_dirs)
  unset(_found_headers)
  unset(_child_dirs)
  unset(_child_hdrs)
  unset(_final_includes)
  unset(_mods)
  unset(_wt_boost_interface_includes)
  unset(_wt_boost_collected_libs)
  unset(_wt_in_tree_boost)
  unset(_wt_libs)

#
# --------- END in-tree detection and filesystem discovery -------------------
#
else()
  #
  # --- No in-tree targets — fall back to CMake's find_package(Boost) ---
  #
  function(find_boost_system)
    find_package(Boost 1.66 QUIET
      COMPONENTS
        system
    )

    set(_BOOST_FOUND ${Boost_FOUND} PARENT_SCOPE)
    if(Boost_VERSION_MAJOR)
      set(_BOOST_MAJOR_VERSION ${Boost_VERSION_MAJOR} PARENT_SCOPE)
      set(_BOOST_MINOR_VERSION ${Boost_VERSION_MINOR} PARENT_SCOPE)
    else()
      set(_BOOST_MAJOR_VERSION ${Boost_MAJOR_VERSION} PARENT_SCOPE)
      set(_BOOST_MINOR_VERSION ${Boost_MINOR_VERSION} PARENT_SCOPE)
    endif()
  endfunction()

  find_boost_system()

  if(_BOOST_FOUND AND _BOOST_MAJOR_VERSION EQUAL 1 AND _BOOST_MINOR_VERSION LESS 69)
    LIST(APPEND Boost_COMPONENTS system)
  endif()

  set(_BOOST_FOUND)
  set(_BOOST_MAJOR_VERSION)
  set(_BOOST_MINOR_VERSION)

  FIND_PACKAGE(Boost 1.66
    COMPONENTS
      ${Boost_COMPONENTS}
    REQUIRED
  )

  # Use IMPORTED targets if defined (map to Wt variables)
  FOREACH(_target ${Boost_COMPONENTS})
    IF(TARGET Boost::${_target})
      STRING(TOUPPER ${_target} _TARGET)
      SET(Boost_${_TARGET}_LIBRARY Boost::${_target})
    ENDIF()
  ENDFOREACH()
  UNSET(_target)
  UNSET(_TARGET)

  IF (Boost_FOUND)
    IF(Boost_VERSION_MAJOR AND NOT Boost_MAJOR_VERSION)
      set(Boost_MAJOR_VERSION ${Boost_VERSION_MAJOR})
    ENDIF()
    IF(Boost_VERSION_MINOR AND NOT Boost_MINOR_VERSION)
      set(Boost_MINOR_VERSION ${Boost_MINOR_VERSION})
    ENDIF()

    # Translate variables from official cmake names to internally used names
    IF(NOT DEFINED BOOST_INCLUDE_DIRS AND DEFINED Boost_INCLUDE_DIRS)
      SET(BOOST_INCLUDE_DIRS ${Boost_INCLUDE_DIRS})
    ENDIF()
    IF(NOT DEFINED BOOST_LIB_DIRS AND DEFINED Boost_LIBRARY_DIRS)
      SET(BOOST_LIB_DIRS ${Boost_LIBRARY_DIRS})
    ENDIF()

    # If find_package actually set variables for individual libs, map them:
    if(DEFINED Boost_FILESYSTEM_LIBRARY)
      SET(BOOST_FS_LIB ${Boost_FILESYSTEM_LIBRARY})
    endif()
    if(DEFINED Boost_PROGRAM_OPTIONS_LIBRARY)
      SET(BOOST_PO_LIB ${Boost_PROGRAM_OPTIONS_LIBRARY})
    endif()
    if(DEFINED Boost_SYSTEM_LIBRARY)
      SET(BOOST_SYSTEM_LIB ${Boost_SYSTEM_LIBRARY})
    endif()
    if(DEFINED Boost_THREAD_LIBRARY)
      SET(BOOST_THREAD_LIB ${Boost_THREAD_LIBRARY})
    endif()
    IF(WIN32 AND BOOST_DYNAMIC AND BUILD_TESTS)
      SET(BOOST_UT_LIB ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
    ELSE()
      IF(NOT DEFINED BOOST_UT_LIB)
        SET(BOOST_UT_LIB "")
      ENDIF()
    ENDIF()

    SET(BOOST_WT_MT_FOUND TRUE)
    SET(BOOST_WT_FOUND TRUE)
    SET(BOOST_WTHTTP_MT_FOUND TRUE)
    SET(BOOST_WTHTTP_FOUND TRUE)

    IF(MSVC AND Boost_MAJOR_VERSION EQUAL 1 AND Boost_MINOR_VERSION LESS 70)
      # use autolink
      SET(BOOST_WT_LIBRARIES "")
      SET(BOOST_WTHTTP_LIBRARIES "")
      SET(BOOST_TEST_LIBRARIES "")
    ELSE()
      # prefer explicit variables if available, otherwise rely on the Boost::<lib> targets we set above
      SET(BOOST_WT_LIBRARIES
          ${Boost_THREAD_LIBRARY}
          ${Boost_SYSTEM_LIBRARY}
          ${Boost_FILESYSTEM_LIBRARY})
      SET(BOOST_WTHTTP_LIBRARIES
          ${Boost_THREAD_LIBRARY}
          ${Boost_PROGRAM_OPTIONS_LIBRARY}
          ${Boost_SYSTEM_LIBRARY}
          ${Boost_FILESYSTEM_LIBRARY})
      SET(BOOST_TEST_LIBRARIES ${BOOST_UT_LIB})
    ENDIF()

    IF(TARGET Boost::headers)
      SET(BOOST_WT_LIBRARIES ${BOOST_WT_LIBRARIES} Boost::headers)
      SET(BOOST_WTHTTP_LIBRARIES ${BOOST_WTHTTP_LIBRARIES} Boost::headers)
    ENDIF()
  ENDIF()
endif()


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ PROGRESS THREE @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

# WtFindBoost.cmake - patched: in-tree Boost detection + filesystem-first include discovery
#
# Behavior:
#  - If Boost::xxx targets already exist (from add_subdirectory(boost)), map them into
#    the BOOST_* variables Wt expects and skip calling find_package()
#  - Additionally, perform a filesystem-first scan for Boost include dirs for modules
#    listed in Boost_COMPONENTS (and BOOST_INCLUDE_LIBRARIES if present).
#  - Ignores generator expressions from target properties and only appends absolute,
#    existing directories to BOOST_INCLUDE_DIRS.
#  - Otherwise fall back to the original find_package(Boost ...) logic.
#
# Based on original WtFindBoost.txt (user-provided). See embedded diagnostics when needed.
#

# default state
SET(BOOST_WT_FOUND FALSE)
SET(BOOST_WT_MT_FOUND FALSE)
SET(BOOST_WTHTTP_FOUND FALSE)
SET(BOOST_WTHTTP_MT_FOUND FALSE)
SET(BOOST_SUPPORT_LIBRARIES "")

IF(NOT DEFINED Boost_ADDITIONAL_VERSIONS)
  SET(Boost_ADDITIONAL_VERSIONS
    "1.66" "1.67" "1.68" "1.69" "1.70" "1.71" "1.72" "1.73" "1.74" "1.75"
    "1.76" "1.77" "1.78" "1.79" "1.80" "1.81" "1.82" "1.83" "1.84" "1.85"
    "1.86" "1.87" "1.88")
ENDIF()

SET(Boost_USE_MULTITHREADED ON)

IF(DEFINED BOOST_PREFIX AND NOT DEFINED BOOST_ROOT)
  SET(BOOST_ROOT ${BOOST_PREFIX})
ENDIF()

IF(WIN32)
  IF(BOOST_DYNAMIC)
    SET(Boost_USE_STATIC_LIBS OFF)
  ELSE()
    SET(Boost_USE_STATIC_LIBS ON)
  ENDIF()
ENDIF()

SET(Boost_DEBUG ON)

# Default components Wt needs (adjust as required)
SET(Boost_COMPONENTS
    program_options
    thread
    asio
    filesystem
    chrono
    system
    numeric
)

IF(WT_CPP17_FILESYSTEM_IMPLEMENTATION STREQUAL "boost")
  LIST(APPEND Boost_COMPONENTS filesystem)
ENDIF()

IF(WIN32 AND BOOST_DYNAMIC AND BUILD_TESTS)
  LIST(APPEND Boost_COMPONENTS unit_test_framework)
ENDIF()

# --------- NEW: Detect an in-tree Boost (targets created by add_subdirectory) ---------
set(_wt_in_tree_boost FALSE)
set(_wt_boost_interface_includes "")
set(_wt_boost_collected_libs "")

foreach(_comp ${Boost_COMPONENTS})
  if(TARGET Boost::${_comp})
    set(_wt_in_tree_boost TRUE)
    list(APPEND _wt_boost_collected_libs Boost::${_comp})
    get_target_property(_incdirs Boost::${_comp} INTERFACE_INCLUDE_DIRECTORIES)
    if(_incdirs)
      list(APPEND _wt_boost_interface_includes ${_incdirs})
    endif()
  endif()
endforeach()

if(_wt_in_tree_boost)
  message(STATUS "WtFindBoost: Boost:: targets detected (in-tree Boost). Using filesystem-first include discovery.")

  set(Boost_FOUND TRUE CACHE INTERNAL "Found in-tree Boost")
  set(BOOST_LIB_DIRS "")

  # Map commonly-used Boost::<lib> targets into Wt variables if present
  set(_wt_libs "")
  foreach(_lib program_options thread system filesystem asio chrono numeric)
    if(TARGET Boost::${_lib})
      list(APPEND _wt_libs Boost::${_lib})
      string(TOUPPER ${_lib} _ULIB)
      set(BOOST_${_ULIB}_LIB Boost::${_lib})
    endif()
  endforeach()

  set(BOOST_WT_LIBRARIES ${_wt_libs})
  set(BOOST_WTHTTP_LIBRARIES ${_wt_libs})

  if(TARGET Boost::headers)
    list(APPEND BOOST_WT_LIBRARIES Boost::headers)
    list(APPEND BOOST_WTHTTP_LIBRARIES Boost::headers)
  endif()

  set(BOOST_WT_FOUND TRUE)
  set(BOOST_WT_MT_FOUND TRUE)
  set(BOOST_WTHTTP_FOUND TRUE)
  set(BOOST_WTHTTP_MT_FOUND TRUE)

  # Filesystem-first include discovery
  set(_collected_includes_raw "")

  set(_mods "")
  if(DEFINED BOOST_INCLUDE_LIBRARIES)
    list(APPEND _mods ${BOOST_INCLUDE_LIBRARIES})
  endif()
  list(APPEND _mods ${Boost_COMPONENTS})
  list(REMOVE_DUPLICATES _mods)

  function(collect_module_includes _moddir _out_var)
    if(NOT _moddir)
      set(${_out_var} "" PARENT_SCOPE)
      return()
    endif()

    set(_raw_list "")

    if(EXISTS "${_moddir}/include" AND IS_DIRECTORY "${_moddir}/include")
      list(APPEND _raw_list "${_moddir}/include")
    endif()

    file(GLOB_RECURSE _found_inc_dirs LIST_DIRECTORIES true "${_moddir}/**/include")
    foreach(_d IN LISTS _found_inc_dirs)
      if(IS_DIRECTORY "${_d}")
        list(APPEND _raw_list "${_d}")
      endif()
    endforeach()

    file(GLOB_RECURSE _found_headers
         "${_moddir}/**/*.hpp"
         "${_moddir}/**/*.h"
         "${_moddir}/**/*.ipp"
         "${_moddir}/**/*.inl")
    foreach(_hdr IN LISTS _found_headers)
      get_filename_component(_hdr_dir "${_hdr}" DIRECTORY)
      if(_hdr_dir AND IS_DIRECTORY "${_hdr_dir}")
        list(APPEND _raw_list "${_hdr_dir}")
      endif()
    endforeach()

    file(GLOB _children LIST_DIRECTORIES true "${_moddir}/*")
    foreach(_child IN LISTS _children)
      if(IS_DIRECTORY "${_child}")
        if(EXISTS "${_child}/include" AND IS_DIRECTORY "${_child}/include")
          list(APPEND _raw_list "${_child}/include")
        endif()
        file(GLOB _child_hdrs "${_child}/*.{hpp,h,ipp,inl}")
        if(_child_hdrs)
          list(APPEND _raw_list "${_child}")
        endif()
      endif()
    endforeach()

    list(REMOVE_DUPLICATES _raw_list)

    set(_noise_regex "/(doc|docs|example|examples|test|tests|bench|benchmark|benchmarks|\\.github|cmake_subdir_test|\\.git)/")

    set(_final_list "")
    foreach(_p IN LISTS _raw_list)
      if(NOT _p)
        continue()
      endif()
      if("${_p}" MATCHES "\\$<")
        continue()
      endif()

      get_filename_component(_abs_p "${_p}" ABSOLUTE)
      if("${_abs_p}" MATCHES "${_noise_regex}")
        continue()
      endif()

      get_filename_component(_basename "${_abs_p}" NAME)
      if(_basename STREQUAL "include")
        if(IS_DIRECTORY "${_abs_p}")
          list(APPEND _final_list "${_abs_p}")
          continue()
        endif()
      endif()

      file(GLOB _hdrs_direct "${_abs_p}/*.{hpp,h,ipp,inl}")
      if(_hdrs_direct)
        list(APPEND _final_list "${_abs_p}")
        continue()
      endif()

      file(GLOB _immediate_children LIST_DIRECTORIES true "${_abs_p}/*")
      set(_accepted FALSE)
      foreach(_ch IN LISTS _immediate_children)
        if(IS_DIRECTORY "${_ch}")
          file(GLOB _child_hdrs "${_ch}/*.{hpp,h,ipp,inl}")
          if(_child_hdrs)
            set(_accepted TRUE)
            break()
          endif()
        endif()
      endforeach()
      if(_accepted)
        list(APPEND _final_list "${_abs_p}")
        continue()
      endif()
    endforeach()

    list(REMOVE_DUPLICATES _final_list)
    set(_out_list "")
    foreach(_q IN LISTS _final_list)
      if(IS_DIRECTORY "${_q}")
        get_filename_component(_realq "${_q}" REALPATH)
        if(_realq AND IS_DIRECTORY "${_realq}")
          list(APPEND _out_list "${_realq}")
        endif()
      endif()
    endforeach()

    list(REMOVE_DUPLICATES _out_list)
    set(${_out_var} "${_out_list}" PARENT_SCOPE)
  endfunction()

  foreach(_mod IN LISTS _mods)
    string(STRIP "${_mod}" _mod_trim)
    if(NOT _mod_trim)
      continue()
    endif()

    if(DEFINED BOOST_ROOT)
      set(_moddir "${BOOST_ROOT}/libs/${_mod_trim}")
    else()
      set(_moddir "${CMAKE_SOURCE_DIR}/boost/libs/${_mod_trim}")
    endif()

    if(NOT EXISTS "${_moddir}")
      set(_moddir_alt "${CMAKE_CURRENT_SOURCE_DIR}/boost/libs/${_mod_trim}")
      if(EXISTS "${_moddir_alt}")
        set(_moddir "${_moddir_alt}")
      endif()
    endif()

    if(NOT EXISTS "${_moddir}")
      message(STATUS "WtFindBoost: module not present, skipping: ${_mod_trim}")
      continue()
    endif()

    collect_module_includes("${_moddir}" _module_incs)
    foreach(_inc IN LISTS _module_incs)
      list(APPEND _collected_includes_raw "${_inc}")
    endforeach()
  endforeach()

  if(DEFINED BOOST_ROOT)
    list(APPEND _collected_includes_raw "${BOOST_ROOT}")
  else()
    list(APPEND _collected_includes_raw "${CMAKE_SOURCE_DIR}/boost")
  endif()

  list(REMOVE_DUPLICATES _collected_includes_raw)

  set(_kept "")
  foreach(_p IN LISTS _collected_includes_raw)
    if(NOT _p)
      continue()
    endif()
    if("${_p}" MATCHES "\\$<.*:.*>")
      continue()
    endif()
    if(IS_DIRECTORY "${_p}")
      collect_module_includes("${_p}" _tmpincs)
      foreach(_i IN LISTS _tmpincs)
        list(APPEND _kept "${_i}")
      endforeach()
    endif()
  endforeach()

  list(REMOVE_DUPLICATES _kept)

  # normalize to absolute realpath list
  set(_abs_includes "")
  foreach(_inc IN LISTS _kept)
    get_filename_component(_abs "${_inc}" REALPATH)
    if(_abs AND IS_DIRECTORY "${_abs}")
      list(APPEND _abs_includes "${_abs}")
    endif()
  endforeach()
  list(REMOVE_DUPLICATES _abs_includes)
  set(BOOST_INCLUDE_DIRS ${_abs_includes} CACHE INTERNAL "Collected in-tree Boost include dirs (filtered)")

  # append to directory and binary properties so build system picks them up
  if(BOOST_INCLUDE_DIRS)
    set_property(DIRECTORY ${CMAKE_SOURCE_DIR} APPEND PROPERTY INCLUDE_DIRECTORIES ${BOOST_INCLUDE_DIRS})
    set_property(DIRECTORY ${CMAKE_BINARY_DIR} APPEND PROPERTY INCLUDE_DIRECTORIES ${BOOST_INCLUDE_DIRS})
    if(NOT CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
      set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} APPEND PROPERTY INCLUDE_DIRECTORIES ${BOOST_INCLUDE_DIRS})
    endif()
    if(NOT CMAKE_CURRENT_BINARY_DIR STREQUAL CMAKE_BINARY_DIR)
      set_property(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} APPEND PROPERTY INCLUDE_DIRECTORIES ${BOOST_INCLUDE_DIRS})
    endif()

    # try attaching to non-alias Boost targets
    foreach(_t IN LISTS BOOST_WT_LIBRARIES)
      if(TARGET ${_t})
        get_target_property(_aliased ${_t} ALIASED_TARGET)
        if(DEFINED _aliased AND NOT "${_aliased}" STREQUAL "NOTFOUND")
          message(STATUS "WtFindBoost: skipping target_include_directories for ALIAS target ${_t} (aliases ${_aliased})")
        else()
          get_target_property(_t_type ${_t} TYPE)
          if(NOT "${_t_type}" STREQUAL "UNKNOWN" AND NOT "${_t_type}" STREQUAL "INTERFACE_LIBRARY")
            message(VERBOSE "WtFindBoost: attaching BOOST_INCLUDE_DIRS to target ${_t}")
            target_include_directories(${_t} INTERFACE ${BOOST_INCLUDE_DIRS})
          else()
            message(STATUS "WtFindBoost: not attaching include dirs to target ${_t} (type=${_t_type})")
          endif()
        endif()
      endif()
    endforeach()

    # attach to project targets as fallback
    get_property(_all_targets GLOBAL PROPERTY TARGETS)
    foreach(_proj_t IN LISTS _all_targets)
      if(NOT TARGET ${_proj_t})
        continue()
      endif()
      if("${_proj_t}" MATCHES "^(Boost::|boost_).*" )
        continue()
      endif()
      get_target_property(_aliased2 ${_proj_t} ALIASED_TARGET)
      if(DEFINED _aliased2 AND NOT "${_aliased2}" STREQUAL "NOTFOUND")
        continue()
      endif()
      get_target_property(_type2 ${_proj_t} TYPE)
      if(DEFINED _type2 AND ("${_type2}" STREQUAL "STATIC_LIBRARY" OR "${_type2}" STREQUAL "SHARED_LIBRARY" OR "${_type2}" STREQUAL "OBJECT_LIBRARY" OR "${_type2}" STREQUAL "MODULE_LIBRARY" OR "${_type2}" STREQUAL "EXECUTABLE"))
        message(VERBOSE "WtFindBoost: attaching BOOST_INCLUDE_DIRS to project target ${_proj_t}")
        target_include_directories(${_proj_t} PRIVATE ${BOOST_INCLUDE_DIRS})
      endif()
    endforeach()

    foreach(_inc IN LISTS BOOST_INCLUDE_DIRS)
      message(STATUS "WtFindBoost: adding include dir => ${_inc}")
    endforeach()
  else()
    message(STATUS "WtFindBoost: no filtered include dirs to add")
    if(_wt_boost_interface_includes)
      message(STATUS "  Note: Boost targets expose INTERFACE include properties (generator-expr); showing them for debugging:")
      foreach(_gi IN LISTS _wt_boost_interface_includes)
        message(STATUS "    ${_gi}")
      endforeach()
    endif()
  endif()

  # Debug summary
  list(LENGTH _collected_includes_raw _len)
  message(STATUS "WtFindBoost: discovered ${_len} physical include directories under boost/libs (pre-filter, showing up to 50):")
  if(_len GREATER 0)
    if(_len LESS 50)
      set(_max_show ${_len})
    else()
      set(_max_show 50)
    endif()
    foreach(_i RANGE 0 ${_max_show})
      if(_i LESS _len)
        list(GET _collected_includes_raw ${_i} _show)
        message(STATUS "  ${_show}")
      endif()
    endforeach()
  endif()

  # Done
  set(BOOST_WT_FOUND TRUE)
  set(BOOST_WT_MT_FOUND TRUE)
  set(BOOST_WTHTTP_FOUND TRUE)
  set(BOOST_WTHTTP_MT_FOUND TRUE)

  unset(_collected_includes_raw)
  unset(_found_inc_dirs)
  unset(_found_headers)
  unset(_child_dirs)
  unset(_child_hdrs)
  unset(_final_includes)
  unset(_mods)
  unset(_wt_boost_interface_includes)
  unset(_wt_boost_collected_libs)
  unset(_wt_in_tree_boost)
  unset(_wt_libs)

else()
  # Fallback to find_package(Boost)
  function(find_boost_system)
    find_package(Boost 1.66 QUIET COMPONENTS system)
    set(_BOOST_FOUND ${Boost_FOUND} PARENT_SCOPE)
    if(Boost_VERSION_MAJOR)
      set(_BOOST_MAJOR_VERSION ${Boost_VERSION_MAJOR} PARENT_SCOPE)
      set(_BOOST_MINOR_VERSION ${Boost_VERSION_MINOR} PARENT_SCOPE)
    else()
      set(_BOOST_MAJOR_VERSION ${Boost_MAJOR_VERSION} PARENT_SCOPE)
      set(_BOOST_MINOR_VERSION ${Boost_MINOR_VERSION} PARENT_SCOPE)
    endif()
  endfunction()

  find_boost_system()

  if(_BOOST_FOUND AND _BOOST_MAJOR_VERSION EQUAL 1 AND _BOOST_MINOR_VERSION LESS 69)
    LIST(APPEND Boost_COMPONENTS system)
  endif()

  set(_BOOST_FOUND)
  set(_BOOST_MAJOR_VERSION)
  set(_BOOST_MINOR_VERSION)

  FIND_PACKAGE(Boost 1.66 COMPONENTS ${Boost_COMPONENTS} REQUIRED)

  FOREACH(_target ${Boost_COMPONENTS})
    IF(TARGET Boost::${_target})
      STRING(TOUPPER ${_target} _TARGET)
      SET(Boost_${_TARGET}_LIBRARY Boost::${_target})
    ENDIF()
  ENDFOREACH()
  UNSET(_target)
  UNSET(_TARGET)

  IF (Boost_FOUND)
    IF(Boost_VERSION_MAJOR AND NOT Boost_MAJOR_VERSION)
      set(Boost_MAJOR_VERSION ${Boost_VERSION_MAJOR})
    ENDIF()
    IF(Boost_VERSION_MINOR AND NOT Boost_MINOR_VERSION)
      set(Boost_MINOR_VERSION ${Boost_MINOR_VERSION})
    ENDIF()

    IF(NOT DEFINED BOOST_INCLUDE_DIRS AND DEFINED Boost_INCLUDE_DIRS)
      SET(BOOST_INCLUDE_DIRS ${Boost_INCLUDE_DIRS})
    ENDIF()
    IF(NOT DEFINED BOOST_LIB_DIRS AND DEFINED Boost_LIBRARY_DIRS)
      SET(BOOST_LIB_DIRS ${Boost_LIBRARY_DIRS})
    ENDIF()

    if(DEFINED Boost_FILESYSTEM_LIBRARY)
      SET(BOOST_FS_LIB ${Boost_FILESYSTEM_LIBRARY})
    endif()
    if(DEFINED Boost_PROGRAM_OPTIONS_LIBRARY)
      SET(BOOST_PO_LIB ${Boost_PROGRAM_OPTIONS_LIBRARY})
    endif()
    if(DEFINED Boost_SYSTEM_LIBRARY)
      SET(BOOST_SYSTEM_LIB ${Boost_SYSTEM_LIBRARY})
    endif()
    if(DEFINED Boost_THREAD_LIBRARY)
      SET(BOOST_THREAD_LIB ${Boost_THREAD_LIBRARY})
    endif()
    IF(WIN32 AND BOOST_DYNAMIC AND BUILD_TESTS)
      SET(BOOST_UT_LIB ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
    ELSE()
      IF(NOT DEFINED BOOST_UT_LIB)
        SET(BOOST_UT_LIB "")
      ENDIF()
    ENDIF()

    SET(BOOST_WT_MT_FOUND TRUE)
    SET(BOOST_WT_FOUND TRUE)
    SET(BOOST_WTHTTP_MT_FOUND TRUE)
    SET(BOOST_WTHTTP_FOUND TRUE)

    IF(MSVC AND Boost_MAJOR_VERSION EQUAL 1 AND Boost_MINOR_VERSION LESS 70)
      SET(BOOST_WT_LIBRARIES "")
      SET(BOOST_WTHTTP_LIBRARIES "")
      SET(BOOST_TEST_LIBRARIES "")
    ELSE()
      SET(BOOST_WT_LIBRARIES
          ${Boost_THREAD_LIBRARY}
          ${Boost_SYSTEM_LIBRARY}
          ${Boost_FILESYSTEM_LIBRARY})
      SET(BOOST_WTHTTP_LIBRARIES
          ${Boost_THREAD_LIBRARY}
          ${Boost_PROGRAM_OPTIONS_LIBRARY}
          ${Boost_SYSTEM_LIBRARY}
          ${Boost_FILESYSTEM_LIBRARY})
      SET(BOOST_TEST_LIBRARIES ${BOOST_UT_LIB})
    ENDIF()

    IF(TARGET Boost::headers)
      SET(BOOST_WT_LIBRARIES ${BOOST_WT_LIBRARIES} Boost::headers)
      SET(BOOST_WTHTTP_LIBRARIES ${BOOST_WTHTTP_LIBRARIES} Boost::headers)
    ENDIF()
  ENDIF()
endif()

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ PROGRESS FOUR @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

# WtFindBoost.cmake - patched: in-tree Boost detection + filesystem-first include discovery
# Improved include scanning and alias-resolving behavior

SET(BOOST_WT_FOUND FALSE)
SET(BOOST_WT_MT_FOUND FALSE)
SET(BOOST_WTHTTP_FOUND FALSE)
SET(BOOST_WTHTTP_MT_FOUND FALSE)
SET(BOOST_SUPPORT_LIBRARIES "")

IF(NOT DEFINED Boost_ADDITIONAL_VERSIONS)
  SET(Boost_ADDITIONAL_VERSIONS
    "1.66" "1.67" "1.68" "1.69" "1.70" "1.71" "1.72" "1.73" "1.74" "1.75"
    "1.76" "1.77" "1.78" "1.79" "1.80" "1.81" "1.82" "1.83" "1.84" "1.85"
    "1.86" "1.87" "1.88")
ENDIF()

SET(Boost_USE_MULTITHREADED ON)

IF(DEFINED BOOST_PREFIX AND NOT DEFINED BOOST_ROOT)
  SET(BOOST_ROOT ${BOOST_PREFIX})
ENDIF()

IF(WIN32)
  IF(BOOST_DYNAMIC)
    SET(Boost_USE_STATIC_LIBS OFF)
  ELSE()
    SET(Boost_USE_STATIC_LIBS ON)
  ENDIF()
ENDIF()

SET(Boost_DEBUG ON)

SET(Boost_COMPONENTS
    program_options
    thread
    asio
    filesystem
    chrono
    system
    numeric
)

IF(WT_CPP17_FILESYSTEM_IMPLEMENTATION STREQUAL "boost")
  LIST(APPEND Boost_COMPONENTS filesystem)
ENDIF()

IF(WIN32 AND BOOST_DYNAMIC AND BUILD_TESTS)
  LIST(APPEND Boost_COMPONENTS unit_test_framework)
ENDIF()

set(_wt_in_tree_boost FALSE)
set(_wt_boost_interface_includes "")
set(_wt_boost_collected_libs "")

foreach(_comp ${Boost_COMPONENTS})
  if(TARGET Boost::${_comp})
    set(_wt_in_tree_boost TRUE)
    list(APPEND _wt_boost_collected_libs Boost::${_comp})
    get_target_property(_incdirs Boost::${_comp} INTERFACE_INCLUDE_DIRECTORIES)
    if(_incdirs)
      list(APPEND _wt_boost_interface_includes ${_incdirs})
    endif()
  endif()
endforeach()

if(_wt_in_tree_boost)
  message(STATUS "WtFindBoost: Boost:: targets detected (in-tree Boost). Using filesystem-first include discovery.")

  set(Boost_FOUND TRUE CACHE INTERNAL "Found in-tree Boost")
  set(BOOST_LIB_DIRS "")

  set(_wt_libs "")
  foreach(_lib program_options thread system filesystem asio chrono numeric)
    if(TARGET Boost::${_lib})
      list(APPEND _wt_libs Boost::${_lib})
      string(TOUPPER ${_lib} _ULIB)
      set(BOOST_${_ULIB}_LIB Boost::${_lib})
    endif()
  endforeach()

  set(BOOST_WT_LIBRARIES ${_wt_libs})
  set(BOOST_WTHTTP_LIBRARIES ${_wt_libs})

  if(TARGET Boost::headers)
    list(APPEND BOOST_WT_LIBRARIES Boost::headers)
    list(APPEND BOOST_WTHTTP_LIBRARIES Boost::headers)
  endif()

  set(BOOST_WT_FOUND TRUE)
  set(BOOST_WT_MT_FOUND TRUE)
  set(BOOST_WTHTTP_FOUND TRUE)
  set(BOOST_WTHTTP_MT_FOUND TRUE)

  # --- Function to collect include directories ---
  function(collect_module_includes _moddir _out_var)
    if(NOT _moddir)
      set(${_out_var} "" PARENT_SCOPE)
      return()
    endif()

    set(_raw_list "")

    if(EXISTS "${_moddir}/include" AND IS_DIRECTORY "${_moddir}/include")
      list(APPEND _raw_list "${_moddir}/include")
    endif()

    file(GLOB_RECURSE _found_inc_dirs LIST_DIRECTORIES true "${_moddir}/**/include")
    foreach(_d IN LISTS _found_inc_dirs)
      if(IS_DIRECTORY "${_d}")
        list(APPEND _raw_list "${_d}")
      endif()
    endforeach()

    file(GLOB_RECURSE _found_headers "${_moddir}/**/*.hpp" "${_moddir}/**/*.h" "${_moddir}/**/*.ipp" "${_moddir}/**/*.inl")
    foreach(_hdr IN LISTS _found_headers)
      get_filename_component(_hdr_dir "${_hdr}" DIRECTORY)
      if(_hdr_dir AND IS_DIRECTORY "${_hdr_dir}")
        list(APPEND _raw_list "${_hdr_dir}")
      endif()
    endforeach()

    file(GLOB _children LIST_DIRECTORIES true "${_moddir}/*")
    foreach(_child IN LISTS _children)
      if(IS_DIRECTORY "${_child}")
        if(EXISTS "${_child}/include" AND IS_DIRECTORY "${_child}/include")
          list(APPEND _raw_list "${_child}/include")
        endif()
        file(GLOB _child_hdrs "${_child}/*.{hpp,h,ipp,inl}")
        if(_child_hdrs)
          list(APPEND _raw_list "${_child}")
        endif()
      endif()
    endforeach()

    list(REMOVE_DUPLICATES _raw_list)

    set(_noise_regex "/(doc|docs|example|examples|test|tests|bench|benchmark|benchmarks|\\.github|cmake_subdir_test|\\.git)/")

    set(_final_list "")
    foreach(_p IN LISTS _raw_list)
      if(NOT _p)
        continue()
      endif()
      if("${_p}" MATCHES "\\$<")
        continue()
      endif()

      get_filename_component(_abs_p "${_p}" ABSOLUTE)

      get_filename_component(_basename "${_abs_p}" NAME)
      if(_basename STREQUAL "include")
        if(IS_DIRECTORY "${_abs_p}")
          list(APPEND _final_list "${_abs_p}")
          continue()
        endif()
      endif()

      if("${_abs_p}" MATCHES "${_noise_regex}")
        continue()
      endif()

      file(GLOB _hdrs_direct "${_abs_p}/*.{hpp,h,ipp,inl}")
      if(_hdrs_direct)
        list(APPEND _final_list "${_abs_p}")
        continue()
      endif()

      file(GLOB _immediate_children LIST_DIRECTORIES true "${_abs_p}/*")
      set(_accepted FALSE)
      foreach(_ch IN LISTS _immediate_children)
        if(IS_DIRECTORY "${_ch}")
          file(GLOB _child_hdrs "${_ch}/*.{hpp,h,ipp,inl}")
          if(_child_hdrs)
            set(_accepted TRUE)
            break()
          endif()
        endif()
      endforeach()
      if(_accepted)
        list(APPEND _final_list "${_abs_p}")
      endif()
    endforeach()

    list(REMOVE_DUPLICATES _final_list)

    set(_out_list "")
    foreach(_q IN LISTS _final_list)
      if(IS_DIRECTORY "${_q}")
        get_filename_component(_realq "${_q}" REALPATH)
        if(_realq AND IS_DIRECTORY "${_realq}")
          list(APPEND _out_list "${_realq}")
        endif()
      endif()
    endforeach()

    list(REMOVE_DUPLICATES _out_list)
    set(${_out_var} "${_out_list}" PARENT_SCOPE)
  endfunction()

  # Scan modules
  set(_collected_includes_raw "")
  foreach(_mod IN LISTS Boost_COMPONENTS)
    string(STRIP "${_mod}" _mod_trim)
    if(NOT _mod_trim)
      continue()
    endif()

    set(_moddir "${CMAKE_SOURCE_DIR}/boost/libs/${_mod_trim}")
    if(NOT EXISTS "${_moddir}")
      set(_moddir_alt "${CMAKE_CURRENT_SOURCE_DIR}/boost/libs/${_mod_trim}")
      if(EXISTS "${_moddir_alt}")
        set(_moddir "${_moddir_alt}")
      endif()
    endif()

    if(NOT EXISTS "${_moddir}")
      message(STATUS "WtFindBoost: module not present, skipping: ${_mod_trim}")
      continue()
    endif()

    collect_module_includes("${_moddir}" _module_incs)
    foreach(_inc IN LISTS _module_incs)
      list(APPEND _collected_includes_raw "${_inc}")
    endforeach()
  endforeach()

  list(REMOVE_DUPLICATES _collected_includes_raw)

  set(BOOST_INCLUDE_DIRS ${_collected_includes_raw} CACHE INTERNAL "Collected in-tree Boost include dirs (filtered)")

  # Attach BOOST_INCLUDE_DIRS to Boost targets — resolve aliases to real targets when possible
  foreach(_t IN LISTS BOOST_WT_LIBRARIES)
    if(NOT _t OR NOT TARGET ${_t})
      continue()
    endif()

    get_target_property(_aliased ${_t} ALIASED_TARGET)
    if(DEFINED _aliased AND NOT "${_aliased}" STREQUAL "NOTFOUND")
      set(_real_target ${_aliased})
    else()
      set(_real_target ${_t})
    endif()

    if(TARGET ${_real_target})
      get_target_property(_real_aliased ${_real_target} ALIASED_TARGET)
      if(DEFINED _real_aliased AND NOT "${_real_aliased}" STREQUAL "NOTFOUND")
        message(STATUS "WtFindBoost: resolved target ${_t} -> ${_real_target} but it is still an ALIAS; skipping attach")
        continue()
      endif()

      get_target_property(_t_type ${_real_target} TYPE)
      if(DEFINED _t_type AND NOT "${_t_type}" STREQUAL "UNKNOWN")
        message(VERBOSE "WtFindBoost: attaching BOOST_INCLUDE_DIRS to resolved target ${_real_target} (from ${_t}) type=${_t_type}")
        target_include_directories(${_real_target} INTERFACE ${BOOST_INCLUDE_DIRS})
      else()
        message(STATUS "WtFindBoost: not attaching include dirs to ${_real_target} (unknown type)")
      endif()
    endif()
  endforeach()

  foreach(_inc IN LISTS BOOST_INCLUDE_DIRS)
    message(STATUS "WtFindBoost: adding include dir => ${_inc}")
  endforeach()
endif()


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ PROGRESS FIVE- BEST ONE @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

# WtFindBoost.cmake - patched: in-tree Boost detection + filesystem-first include discovery
# Improved include scanning and alias-resolving behavior

SET(BOOST_WT_FOUND FALSE)
SET(BOOST_WT_MT_FOUND FALSE)
SET(BOOST_WTHTTP_FOUND FALSE)
SET(BOOST_WTHTTP_MT_FOUND FALSE)
SET(BOOST_SUPPORT_LIBRARIES "")

IF(NOT DEFINED Boost_ADDITIONAL_VERSIONS)
  SET(Boost_ADDITIONAL_VERSIONS
    "1.66" "1.67" "1.68" "1.69" "1.70" "1.71" "1.72" "1.73" "1.74" "1.75"
    "1.76" "1.77" "1.78" "1.79" "1.80" "1.81" "1.82" "1.83" "1.84" "1.85"
    "1.86" "1.87" "1.88")
ENDIF()

SET(Boost_USE_MULTITHREADED ON)

IF(DEFINED BOOST_PREFIX AND NOT DEFINED BOOST_ROOT)
  SET(BOOST_ROOT ${BOOST_PREFIX})
ENDIF()

IF(WIN32)
  IF(BOOST_DYNAMIC)
    SET(Boost_USE_STATIC_LIBS OFF)
  ELSE()
    SET(Boost_USE_STATIC_LIBS ON)
  ENDIF()
ENDIF()

SET(Boost_DEBUG ON)

SET(Boost_COMPONENTS
    program_options
    thread
    asio
    filesystem
    chrono
    system
    numeric
)

IF(WT_CPP17_FILESYSTEM_IMPLEMENTATION STREQUAL "boost")
  LIST(APPEND Boost_COMPONENTS filesystem)
ENDIF()

IF(WIN32 AND BOOST_DYNAMIC AND BUILD_TESTS)
  LIST(APPEND Boost_COMPONENTS unit_test_framework)
ENDIF()

set(_wt_in_tree_boost FALSE)
set(_wt_boost_interface_includes "")
set(_wt_boost_collected_libs "")

foreach(_comp ${Boost_COMPONENTS})
  if(TARGET Boost::${_comp})
    set(_wt_in_tree_boost TRUE)
    list(APPEND _wt_boost_collected_libs Boost::${_comp})
    get_target_property(_incdirs Boost::${_comp} INTERFACE_INCLUDE_DIRECTORIES)
    if(_incdirs)
      list(APPEND _wt_boost_interface_includes ${_incdirs})
    endif()
  endif()
endforeach()

if(_wt_in_tree_boost)
  message(STATUS "WtFindBoost: Boost:: targets detected (in-tree Boost). Using filesystem-first include discovery.")

  set(Boost_FOUND TRUE CACHE INTERNAL "Found in-tree Boost")
  set(BOOST_LIB_DIRS "")

  set(_wt_libs "")
  foreach(_lib program_options thread system filesystem asio chrono numeric)
    if(TARGET Boost::${_lib})
      list(APPEND _wt_libs Boost::${_lib})
      string(TOUPPER ${_lib} _ULIB)
      set(BOOST_${_ULIB}_LIB Boost::${_lib})
    endif()
  endforeach()

  set(BOOST_WT_LIBRARIES ${_wt_libs})
  set(BOOST_WTHTTP_LIBRARIES ${_wt_libs})

  if(TARGET Boost::headers)
    list(APPEND BOOST_WT_LIBRARIES Boost::headers)
    list(APPEND BOOST_WTHTTP_LIBRARIES Boost::headers)
  endif()

  set(BOOST_WT_FOUND TRUE)
  set(BOOST_WT_MT_FOUND TRUE)
  set(BOOST_WTHTTP_FOUND TRUE)
  set(BOOST_WTHTTP_MT_FOUND TRUE)

  # --- Function to collect include directories ---
  function(collect_module_includes _moddir _out_var)
    if(NOT _moddir)
      set(${_out_var} "" PARENT_SCOPE)
      return()
    endif()

    set(_raw_list "")

    if(EXISTS "${_moddir}/include" AND IS_DIRECTORY "${_moddir}/include")
      list(APPEND _raw_list "${_moddir}/include")
    endif()

    file(GLOB_RECURSE _found_inc_dirs LIST_DIRECTORIES true "${_moddir}/**/include")
    foreach(_d IN LISTS _found_inc_dirs)
      if(IS_DIRECTORY "${_d}")
        list(APPEND _raw_list "${_d}")
      endif()
    endforeach()

    file(GLOB_RECURSE _found_headers "${_moddir}/**/*.hpp" "${_moddir}/**/*.h" "${_moddir}/**/*.ipp" "${_moddir}/**/*.inl")
    foreach(_hdr IN LISTS _found_headers)
      get_filename_component(_hdr_dir "${_hdr}" DIRECTORY)
      if(_hdr_dir AND IS_DIRECTORY "${_hdr_dir}")
        list(APPEND _raw_list "${_hdr_dir}")
      endif()
    endforeach()

    file(GLOB _children LIST_DIRECTORIES true "${_moddir}/*")
    foreach(_child IN LISTS _children)
      if(IS_DIRECTORY "${_child}")
        if(EXISTS "${_child}/include" AND IS_DIRECTORY "${_child}/include")
          list(APPEND _raw_list "${_child}/include")
        endif()
        file(GLOB _child_hdrs "${_child}/*.{hpp,h,ipp,inl}")
        if(_child_hdrs)
          list(APPEND _raw_list "${_child}")
        endif()
      endif()
    endforeach()

    list(REMOVE_DUPLICATES _raw_list)

    set(_noise_regex "/(doc|docs|example|examples|test|tests|bench|benchmark|benchmarks|\\.github|cmake_subdir_test|\\.git)/")

    set(_final_list "")
    foreach(_p IN LISTS _raw_list)
      if(NOT _p)
        continue()
      endif()
      if("${_p}" MATCHES "\\$<")
        continue()
      endif()

      get_filename_component(_abs_p "${_p}" ABSOLUTE)

      get_filename_component(_basename "${_abs_p}" NAME)
      if(_basename STREQUAL "include")
        if(IS_DIRECTORY "${_abs_p}")
          list(APPEND _final_list "${_abs_p}")
          continue()
        endif()
      endif()

      if("${_abs_p}" MATCHES "${_noise_regex}")
        continue()
      endif()

      file(GLOB _hdrs_direct "${_abs_p}/*.{hpp,h,ipp,inl}")
      if(_hdrs_direct)
        list(APPEND _final_list "${_abs_p}")
        continue()
      endif()

      file(GLOB _immediate_children LIST_DIRECTORIES true "${_abs_p}/*")
      set(_accepted FALSE)
      foreach(_ch IN LISTS _immediate_children)
        if(IS_DIRECTORY "${_ch}")
          file(GLOB _child_hdrs "${_ch}/*.{hpp,h,ipp,inl}")
          if(_child_hdrs)
            set(_accepted TRUE)
            break()
          endif()
        endif()
      endforeach()
      if(_accepted)
        list(APPEND _final_list "${_abs_p}")
      endif()
    endforeach()

    list(REMOVE_DUPLICATES _final_list)

    set(_out_list "")
    foreach(_q IN LISTS _final_list)
      if(IS_DIRECTORY "${_q}")
        get_filename_component(_realq "${_q}" REALPATH)
        if(_realq AND IS_DIRECTORY "${_realq}")
          list(APPEND _out_list "${_realq}")
        endif()
      endif()
    endforeach()

    list(REMOVE_DUPLICATES _out_list)
    set(${_out_var} "${_out_list}" PARENT_SCOPE)
  endfunction()

  # Scan modules
  set(_collected_includes_raw "")
  foreach(_mod IN LISTS Boost_COMPONENTS)
    string(STRIP "${_mod}" _mod_trim)
    if(NOT _mod_trim)
      continue()
    endif()

    set(_moddir "${CMAKE_SOURCE_DIR}/boost/libs/${_mod_trim}")
    if(NOT EXISTS "${_moddir}")
      set(_moddir_alt "${CMAKE_CURRENT_SOURCE_DIR}/boost/libs/${_mod_trim}")
      if(EXISTS "${_moddir_alt}")
        set(_moddir "${_moddir_alt}")
      endif()
    endif()

    if(NOT EXISTS "${_moddir}")
      message(STATUS "WtFindBoost: module not present, skipping: ${_mod_trim}")
      continue()
    endif()

    collect_module_includes("${_moddir}" _module_incs)
    foreach(_inc IN LISTS _module_incs)
      list(APPEND _collected_includes_raw "${_inc}")
    endforeach()
  endforeach()

  list(REMOVE_DUPLICATES _collected_includes_raw)

  set(BOOST_INCLUDE_DIRS ${_collected_includes_raw} CACHE INTERNAL "Collected in-tree Boost include dirs (filtered)")

  # Attach BOOST_INCLUDE_DIRS to Boost targets — resolve aliases to real targets when possible
  foreach(_t IN LISTS BOOST_WT_LIBRARIES)
    if(NOT _t OR NOT TARGET ${_t})
      continue()
    endif()

    get_target_property(_aliased ${_t} ALIASED_TARGET)
    if(DEFINED _aliased AND NOT "${_aliased}" STREQUAL "NOTFOUND")
      set(_real_target ${_aliased})
    else()
      set(_real_target ${_t})
    endif()

    if(TARGET ${_real_target})
      get_target_property(_real_aliased ${_real_target} ALIASED_TARGET)
      if(DEFINED _real_aliased AND NOT "${_real_aliased}" STREQUAL "NOTFOUND")
        message(STATUS "WtFindBoost: resolved target ${_t} -> ${_real_target} but it is still an ALIAS; skipping attach")
        continue()
      endif()

      get_target_property(_t_type ${_real_target} TYPE)
      if(DEFINED _t_type AND NOT "${_t_type}" STREQUAL "UNKNOWN")
        message(VERBOSE "WtFindBoost: attaching BOOST_INCLUDE_DIRS to resolved target ${_real_target} (from ${_t}) type=${_t_type}")
        target_include_directories(${_real_target} INTERFACE ${BOOST_INCLUDE_DIRS})
      else()
        message(STATUS "WtFindBoost: not attaching include dirs to ${_real_target} (unknown type)")
      endif()
    endif()
  endforeach()

  foreach(_inc IN LISTS BOOST_INCLUDE_DIRS)
    message(STATUS "WtFindBoost: adding include dir => ${_inc}")
  endforeach()
endif()

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ PROGRESS FIVE- BEST ONE @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

# WtFindBoost.cmake - patched: in-tree Boost detection + filesystem-first include discovery
# Improved include scanning and alias-resolving behavior

SET(BOOST_WT_FOUND FALSE)
SET(BOOST_WT_MT_FOUND FALSE)
SET(BOOST_WTHTTP_FOUND FALSE)
SET(BOOST_WTHTTP_MT_FOUND FALSE)
SET(BOOST_SUPPORT_LIBRARIES "")

IF(NOT DEFINED Boost_ADDITIONAL_VERSIONS)
  SET(Boost_ADDITIONAL_VERSIONS
    "1.66" "1.67" "1.68" "1.69" "1.70" "1.71" "1.72" "1.73" "1.74" "1.75"
    "1.76" "1.77" "1.78" "1.79" "1.80" "1.81" "1.82" "1.83" "1.84" "1.85"
    "1.86" "1.87" "1.88")
ENDIF()

SET(Boost_USE_MULTITHREADED ON)

IF(DEFINED BOOST_PREFIX AND NOT DEFINED BOOST_ROOT)
  SET(BOOST_ROOT ${BOOST_PREFIX})
ENDIF()

IF(WIN32)
  IF(BOOST_DYNAMIC)
    SET(Boost_USE_STATIC_LIBS OFF)
  ELSE()
    SET(Boost_USE_STATIC_LIBS ON)
  ENDIF()
ENDIF()

SET(Boost_DEBUG ON)

SET(Boost_COMPONENTS
    program_options
    thread
    asio
    filesystem
    chrono
    system
    numeric
    serialization
    spirit
    array
)

IF(WT_CPP17_FILESYSTEM_IMPLEMENTATION STREQUAL "boost")
  LIST(APPEND Boost_COMPONENTS filesystem)
ENDIF()

IF(WIN32 AND BOOST_DYNAMIC AND BUILD_TESTS)
  LIST(APPEND Boost_COMPONENTS unit_test_framework)
ENDIF()

set(_wt_in_tree_boost FALSE)
set(_wt_boost_interface_includes "")
set(_wt_boost_collected_libs "")

foreach(_comp ${Boost_COMPONENTS})
  if(TARGET Boost::${_comp})
    set(_wt_in_tree_boost TRUE)
    list(APPEND _wt_boost_collected_libs Boost::${_comp})
    get_target_property(_incdirs Boost::${_comp} INTERFACE_INCLUDE_DIRECTORIES)
    if(_incdirs)
      list(APPEND _wt_boost_interface_includes ${_incdirs})
    endif()
  endif()
endforeach()

if(_wt_in_tree_boost)
  message(STATUS "WtFindBoost: Boost:: targets detected (in-tree Boost). Using filesystem-first include discovery.")

  set(Boost_FOUND TRUE CACHE INTERNAL "Found in-tree Boost")
  set(BOOST_LIB_DIRS "")

  set(_wt_libs "")
  foreach(_lib program_options thread system filesystem asio chrono numeric serialization spirit array)
    if(TARGET Boost::${_lib})
      list(APPEND _wt_libs Boost::${_lib})
      string(TOUPPER ${_lib} _ULIB)
      set(BOOST_${_ULIB}_LIB Boost::${_lib})
    endif()
  endforeach()

  set(BOOST_WT_LIBRARIES ${_wt_libs})
  set(BOOST_WTHTTP_LIBRARIES ${_wt_libs})

  if(TARGET Boost::headers)
    list(APPEND BOOST_WT_LIBRARIES Boost::headers)
    list(APPEND BOOST_WTHTTP_LIBRARIES Boost::headers)
  endif()

  set(BOOST_WT_FOUND TRUE)
  set(BOOST_WT_MT_FOUND TRUE)
  set(BOOST_WTHTTP_FOUND TRUE)
  set(BOOST_WTHTTP_MT_FOUND TRUE)

  # --- Function to collect include directories ---
  function(collect_module_includes _moddir _out_var)
    if(NOT _moddir)
      set(${_out_var} "" PARENT_SCOPE)
      return()
    endif()

    set(_raw_list "")

    if(EXISTS "${_moddir}/include" AND IS_DIRECTORY "${_moddir}/include")
      list(APPEND _raw_list "${_moddir}/include")
    endif()

    file(GLOB_RECURSE _found_inc_dirs LIST_DIRECTORIES true "${_moddir}/**/include")
    foreach(_d IN LISTS _found_inc_dirs)
      if(IS_DIRECTORY "${_d}")
        list(APPEND _raw_list "${_d}")
      endif()
    endforeach()

    file(GLOB_RECURSE _found_headers "${_moddir}/**/*.hpp" "${_moddir}/**/*.h" "${_moddir}/**/*.ipp" "${_moddir}/**/*.inl")
    foreach(_hdr IN LISTS _found_headers)
      get_filename_component(_hdr_dir "${_hdr}" DIRECTORY)
      if(_hdr_dir AND IS_DIRECTORY "${_hdr_dir}")
        list(APPEND _raw_list "${_hdr_dir}")
      endif()
    endforeach()

    file(GLOB _children LIST_DIRECTORIES true "${_moddir}/*")
    foreach(_child IN LISTS _children)
      if(IS_DIRECTORY "${_child}")
        if(EXISTS "${_child}/include" AND IS_DIRECTORY "${_child}/include")
          list(APPEND _raw_list "${_child}/include")
        endif()
        file(GLOB _child_hdrs "${_child}/*.{hpp,h,ipp,inl}")
        if(_child_hdrs)
          list(APPEND _raw_list "${_child}")
        endif()
      endif()
    endforeach()

    list(REMOVE_DUPLICATES _raw_list)

    set(_noise_regex "/(doc|docs|example|examples|test|tests|bench|benchmark|benchmarks|\\.github|cmake_subdir_test|\\.git)/")

    set(_final_list "")
    foreach(_p IN LISTS _raw_list)
      if(NOT _p)
        continue()
      endif()
      if("${_p}" MATCHES "\\$<")
        continue()
      endif()

      get_filename_component(_abs_p "${_p}" ABSOLUTE)

      get_filename_component(_basename "${_abs_p}" NAME)
      if(_basename STREQUAL "include")
        if(IS_DIRECTORY "${_abs_p}")
          list(APPEND _final_list "${_abs_p}")
          continue()
        endif()
      endif()

      if("${_abs_p}" MATCHES "${_noise_regex}")
        continue()
      endif()

      file(GLOB _hdrs_direct "${_abs_p}/*.{hpp,h,ipp,inl}")
      if(_hdrs_direct)
        list(APPEND _final_list "${_abs_p}")
        continue()
      endif()

      file(GLOB _immediate_children LIST_DIRECTORIES true "${_abs_p}/*")
      set(_accepted FALSE)
      foreach(_ch IN LISTS _immediate_children)
        if(IS_DIRECTORY "${_ch}")
          file(GLOB _child_hdrs "${_ch}/*.{hpp,h,ipp,inl}")
          if(_child_hdrs)
            set(_accepted TRUE)
            break()
          endif()
        endif()
      endforeach()
      if(_accepted)
        list(APPEND _final_list "${_abs_p}")
      endif()
    endforeach()

    list(REMOVE_DUPLICATES _final_list)

    set(_out_list "")
    foreach(_q IN LISTS _final_list)
      if(IS_DIRECTORY "${_q}")
        get_filename_component(_realq "${_q}" REALPATH)
        if(_realq AND IS_DIRECTORY "${_realq}")
          list(APPEND _out_list "${_realq}")
        endif()
      endif()
    endforeach()

    list(REMOVE_DUPLICATES _out_list)
    set(${_out_var} "${_out_list}" PARENT_SCOPE)
  endfunction()

  # Scan modules
  set(_collected_includes_raw "")
  foreach(_mod IN LISTS Boost_COMPONENTS)
    string(STRIP "${_mod}" _mod_trim)
    if(NOT _mod_trim)
      continue()
    endif()

    set(_moddir "${CMAKE_SOURCE_DIR}/boost/libs/${_mod_trim}")
    if(NOT EXISTS "${_moddir}")
      set(_moddir_alt "${CMAKE_CURRENT_SOURCE_DIR}/boost/libs/${_mod_trim}")
      if(EXISTS "${_moddir_alt}")
        set(_moddir "${_moddir_alt}")
      endif()
    endif()

    if(NOT EXISTS "${_moddir}")
      message(STATUS "WtFindBoost: module not present, skipping: ${_mod_trim}")
      continue()
    endif()

    collect_module_includes("${_moddir}" _module_incs)
    foreach(_inc IN LISTS _module_incs)
      list(APPEND _collected_includes_raw "${_inc}")
    endforeach()
  endforeach()

  list(REMOVE_DUPLICATES _collected_includes_raw)

  set(BOOST_INCLUDE_DIRS ${_collected_includes_raw} CACHE INTERNAL "Collected in-tree Boost include dirs (filtered)")

  # Attach BOOST_INCLUDE_DIRS to Boost targets — resolve aliases to real targets when possible
  foreach(_t IN LISTS BOOST_WT_LIBRARIES)
    if(NOT _t OR NOT TARGET ${_t})
      continue()
    endif()

    get_target_property(_aliased ${_t} ALIASED_TARGET)
    if(DEFINED _aliased AND NOT "${_aliased}" STREQUAL "NOTFOUND")
      set(_real_target ${_aliased})
    else()
      set(_real_target ${_t})
    endif()

    if(TARGET ${_real_target})
      get_target_property(_real_aliased ${_real_target} ALIASED_TARGET)
      if(DEFINED _real_aliased AND NOT "${_real_aliased}" STREQUAL "NOTFOUND")
        message(STATUS "WtFindBoost: resolved target ${_t} -> ${_real_target} but it is still an ALIAS; skipping attach")
        continue()
      endif()

      get_target_property(_t_type ${_real_target} TYPE)
      if(DEFINED _t_type AND NOT "${_t_type}" STREQUAL "UNKNOWN")
        message(VERBOSE "WtFindBoost: attaching BOOST_INCLUDE_DIRS to resolved target ${_real_target} (from ${_t}) type=${_t_type}")
        target_include_directories(${_real_target} INTERFACE ${BOOST_INCLUDE_DIRS})
      else()
        message(STATUS "WtFindBoost: not attaching include dirs to ${_real_target} (unknown type)")
      endif()
    endif()
  endforeach()

  foreach(_inc IN LISTS BOOST_INCLUDE_DIRS)
    message(STATUS "WtFindBoost: adding include dir => ${_inc}")
  endforeach()
endif()


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ PROGRESS FIVE- BEST TWO @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

# WtFindBoost.cmake - patched: in-tree Boost detection + filesystem-first include discovery
# Improved include scanning and alias-resolving behavior

SET(BOOST_WT_FOUND FALSE)
SET(BOOST_WT_MT_FOUND FALSE)
SET(BOOST_WTHTTP_FOUND FALSE)
SET(BOOST_WTHTTP_MT_FOUND FALSE)
SET(BOOST_SUPPORT_LIBRARIES "")

IF(NOT DEFINED Boost_ADDITIONAL_VERSIONS)
  SET(Boost_ADDITIONAL_VERSIONS
    "1.66" "1.67" "1.68" "1.69" "1.70" "1.71" "1.72" "1.73" "1.74" "1.75"
    "1.76" "1.77" "1.78" "1.79" "1.80" "1.81" "1.82" "1.83" "1.84" "1.85"
    "1.86" "1.87" "1.88")
ENDIF()

SET(Boost_USE_MULTITHREADED ON)

IF(DEFINED BOOST_PREFIX AND NOT DEFINED BOOST_ROOT)
  SET(BOOST_ROOT ${BOOST_PREFIX})
ENDIF()

IF(WIN32)
  IF(BOOST_DYNAMIC)
    SET(Boost_USE_STATIC_LIBS OFF)
  ELSE()
    SET(Boost_USE_STATIC_LIBS ON)
  ENDIF()
ENDIF()

SET(Boost_DEBUG ON)

SET(Boost_COMPONENTS
    program_options
    thread
    asio
    filesystem
    chrono
    system
    numeric
    serialization
    spirit
    array
)

IF(WT_CPP17_FILESYSTEM_IMPLEMENTATION STREQUAL "boost")
  LIST(APPEND Boost_COMPONENTS filesystem)
ENDIF()

IF(WIN32 AND BOOST_DYNAMIC AND BUILD_TESTS)
  LIST(APPEND Boost_COMPONENTS unit_test_framework)
ENDIF()

set(_wt_in_tree_boost FALSE)
set(_wt_boost_interface_includes "")
set(_wt_boost_collected_libs "")

foreach(_comp ${Boost_COMPONENTS})
  if(TARGET Boost::${_comp})
    set(_wt_in_tree_boost TRUE)
    list(APPEND _wt_boost_collected_libs Boost::${_comp})
    get_target_property(_incdirs Boost::${_comp} INTERFACE_INCLUDE_DIRECTORIES)
    if(_incdirs)
      list(APPEND _wt_boost_interface_includes ${_incdirs})
    endif()
  endif()
endforeach()

if(_wt_in_tree_boost)
  message(STATUS "WtFindBoost: Boost:: targets detected (in-tree Boost). Using filesystem-first include discovery.")

  set(Boost_FOUND TRUE CACHE INTERNAL "Found in-tree Boost")
  set(BOOST_LIB_DIRS "")

  set(_wt_libs "")
  foreach(_lib program_options thread system filesystem asio chrono numeric serialization spirit array)
    if(TARGET Boost::${_lib})
      list(APPEND _wt_libs Boost::${_lib})
      string(TOUPPER ${_lib} _ULIB)
      set(BOOST_${_ULIB}_LIB Boost::${_lib})
    endif()
  endforeach()

  set(BOOST_WT_LIBRARIES ${_wt_libs})
  set(BOOST_WTHTTP_LIBRARIES ${_wt_libs})

  if(TARGET Boost::headers)
    list(APPEND BOOST_WT_LIBRARIES Boost::headers)
    list(APPEND BOOST_WTHTTP_LIBRARIES Boost::headers)
  endif()

  set(BOOST_WT_FOUND TRUE)
  set(BOOST_WT_MT_FOUND TRUE)
  set(BOOST_WTHTTP_FOUND TRUE)
  set(BOOST_WTHTTP_MT_FOUND TRUE)

  # --- Function to collect include directories ---
  function(collect_module_includes _moddir _out_var)
    if(NOT _moddir)
      set(${_out_var} "" PARENT_SCOPE)
      return()
    endif()

    set(_raw_list "")

    if(EXISTS "${_moddir}/include" AND IS_DIRECTORY "${_moddir}/include")
      list(APPEND _raw_list "${_moddir}/include")
    endif()

    file(GLOB_RECURSE _found_inc_dirs LIST_DIRECTORIES true "${_moddir}/**/include")
    foreach(_d IN LISTS _found_inc_dirs)
      if(IS_DIRECTORY "${_d}")
        list(APPEND _raw_list "${_d}")
      endif()
    endforeach()

    file(GLOB_RECURSE _found_headers "${_moddir}/**/*.hpp" "${_moddir}/**/*.h" "${_moddir}/**/*.ipp" "${_moddir}/**/*.inl")
    foreach(_hdr IN LISTS _found_headers)
      get_filename_component(_hdr_dir "${_hdr}" DIRECTORY)
      if(_hdr_dir AND IS_DIRECTORY "${_hdr_dir}")
        list(APPEND _raw_list "${_hdr_dir}")
      endif()
    endforeach()

    file(GLOB _children LIST_DIRECTORIES true "${_moddir}/*")
    foreach(_child IN LISTS _children)
      if(IS_DIRECTORY "${_child}")
        if(EXISTS "${_child}/include" AND IS_DIRECTORY "${_child}/include")
          list(APPEND _raw_list "${_child}/include")
        endif()
        file(GLOB _child_hdrs "${_child}/*.{hpp,h,ipp,inl}")
        if(_child_hdrs)
          list(APPEND _raw_list "${_child}")
        endif()
      endif()
    endforeach()

    list(REMOVE_DUPLICATES _raw_list)

    set(_noise_regex "/(doc|docs|example|examples|test|tests|bench|benchmark|benchmarks|\\.github|cmake_subdir_test|\\.git)/")

    set(_final_list "")
    foreach(_p IN LISTS _raw_list)
      if(NOT _p)
        continue()
      endif()
      if("${_p}" MATCHES "\\$<")
        continue()
      endif()

      get_filename_component(_abs_p "${_p}" ABSOLUTE)

      get_filename_component(_basename "${_abs_p}" NAME)
      if(_basename STREQUAL "include")
        if(IS_DIRECTORY "${_abs_p}")
          list(APPEND _final_list "${_abs_p}")
          continue()
        endif()
      endif()

      if("${_abs_p}" MATCHES "${_noise_regex}")
        continue()
      endif()

      file(GLOB _hdrs_direct "${_abs_p}/*.{hpp,h,ipp,inl}")
      if(_hdrs_direct)
        list(APPEND _final_list "${_abs_p}")
        continue()
      endif()

      file(GLOB _immediate_children LIST_DIRECTORIES true "${_abs_p}/*")
      set(_accepted FALSE)
      foreach(_ch IN LISTS _immediate_children)
        if(IS_DIRECTORY "${_ch}")
          file(GLOB _child_hdrs "${_ch}/*.{hpp,h,ipp,inl}")
          if(_child_hdrs)
            set(_accepted TRUE)
            break()
          endif()
        endif()
      endforeach()
      if(_accepted)
        list(APPEND _final_list "${_abs_p}")
      endif()
    endforeach()

    list(REMOVE_DUPLICATES _final_list)

    set(_out_list "")
    foreach(_q IN LISTS _final_list)
      if(IS_DIRECTORY "${_q}")
        get_filename_component(_realq "${_q}" REALPATH)
        if(_realq AND IS_DIRECTORY "${_realq}")
          list(APPEND _out_list "${_realq}")
        endif()
      endif()
    endforeach()

    list(REMOVE_DUPLICATES _out_list)
    set(${_out_var} "${_out_list}" PARENT_SCOPE)
  endfunction()

  # Scan modules
  set(_collected_includes_raw "")
  foreach(_mod IN LISTS Boost_COMPONENTS)
    string(STRIP "${_mod}" _mod_trim)
    if(NOT _mod_trim)
      continue()
    endif()

    set(_moddir "${CMAKE_SOURCE_DIR}/boost/libs/${_mod_trim}")
    if(NOT EXISTS "${_moddir}")
      set(_moddir_alt "${CMAKE_CURRENT_SOURCE_DIR}/boost/libs/${_mod_trim}")
      if(EXISTS "${_moddir_alt}")
        set(_moddir "${_moddir_alt}")
      endif()
    endif()

    if(NOT EXISTS "${_moddir}")
      message(STATUS "WtFindBoost: module not present, skipping: ${_mod_trim}")
      continue()
    endif()

    collect_module_includes("${_moddir}" _module_incs)
    foreach(_inc IN LISTS _module_incs)
      list(APPEND _collected_includes_raw "${_inc}")
    endforeach()
  endforeach()

  list(REMOVE_DUPLICATES _collected_includes_raw)

  set(BOOST_INCLUDE_DIRS ${_collected_includes_raw} CACHE INTERNAL "Collected in-tree Boost include dirs (filtered)")

  # Attach BOOST_INCLUDE_DIRS to Boost targets — resolve aliases to real targets when possible
  foreach(_t IN LISTS BOOST_WT_LIBRARIES)
    if(NOT _t OR NOT TARGET ${_t})
      continue()
    endif()

    get_target_property(_aliased ${_t} ALIASED_TARGET)
    if(DEFINED _aliased AND NOT "${_aliased}" STREQUAL "NOTFOUND")
      set(_real_target ${_aliased})
    else()
      set(_real_target ${_t})
    endif()

    if(TARGET ${_real_target})
      get_target_property(_real_aliased ${_real_target} ALIASED_TARGET)
      if(DEFINED _real_aliased AND NOT "${_real_aliased}" STREQUAL "NOTFOUND")
        message(STATUS "WtFindBoost: resolved target ${_t} -> ${_real_target} but it is still an ALIAS; skipping attach")
        continue()
      endif()

      get_target_property(_t_type ${_real_target} TYPE)
      if(DEFINED _t_type AND NOT "${_t_type}" STREQUAL "UNKNOWN")
        message(VERBOSE "WtFindBoost: attaching BOOST_INCLUDE_DIRS to resolved target ${_real_target} (from ${_t}) type=${_t_type}")
        target_include_directories(${_real_target} INTERFACE ${BOOST_INCLUDE_DIRS})
      else()
        message(STATUS "WtFindBoost: not attaching include dirs to ${_real_target} (unknown type)")
      endif()
    endif()
  endforeach()

  foreach(_inc IN LISTS BOOST_INCLUDE_DIRS)
    message(STATUS "WtFindBoost: adding include dir => ${_inc}")
  endforeach()
endif()

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ PROGRESS FIVE- BEST THREE @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

# WtFindBoost.cmake - patched: in-tree Boost detection + filesystem-first include discovery
# Improved include scanning and alias-resolving behavior

SET(BOOST_WT_FOUND FALSE)
SET(BOOST_WT_MT_FOUND FALSE)
SET(BOOST_WTHTTP_FOUND FALSE)
SET(BOOST_WTHTTP_MT_FOUND FALSE)
SET(BOOST_SUPPORT_LIBRARIES "")

IF(NOT DEFINED Boost_ADDITIONAL_VERSIONS)
  SET(Boost_ADDITIONAL_VERSIONS
    "1.66" "1.67" "1.68" "1.69" "1.70" "1.71" "1.72" "1.73" "1.74" "1.75"
    "1.76" "1.77" "1.78" "1.79" "1.80" "1.81" "1.82" "1.83" "1.84" "1.85"
    "1.86" "1.87" "1.88")
ENDIF()

SET(Boost_USE_MULTITHREADED ON)

IF(DEFINED BOOST_PREFIX AND NOT DEFINED BOOST_ROOT)
  SET(BOOST_ROOT ${BOOST_PREFIX})
ENDIF()

IF(WIN32)
  IF(BOOST_DYNAMIC)
    SET(Boost_USE_STATIC_LIBS OFF)
  ELSE()
    SET(Boost_USE_STATIC_LIBS ON)
  ENDIF()
ENDIF()

SET(Boost_DEBUG ON)

SET(Boost_COMPONENTS
    program_options
    thread
    asio
    filesystem
    chrono
    system
    numeric
	serialization
	spirit
	array
	interprocess
)

IF(WT_CPP17_FILESYSTEM_IMPLEMENTATION STREQUAL "boost")
  LIST(APPEND Boost_COMPONENTS filesystem)
ENDIF()

IF(WIN32 AND BOOST_DYNAMIC AND BUILD_TESTS)
  LIST(APPEND Boost_COMPONENTS unit_test_framework)
ENDIF()

set(_wt_in_tree_boost FALSE)
set(_wt_boost_interface_includes "")
set(_wt_boost_collected_libs "")

foreach(_comp ${Boost_COMPONENTS})
  if(TARGET Boost::${_comp})
    set(_wt_in_tree_boost TRUE)
    list(APPEND _wt_boost_collected_libs Boost::${_comp})
    get_target_property(_incdirs Boost::${_comp} INTERFACE_INCLUDE_DIRECTORIES)
    if(_incdirs)
      list(APPEND _wt_boost_interface_includes ${_incdirs})
    endif()
  endif()
endforeach()

if(_wt_in_tree_boost)
  message(STATUS "WtFindBoost: Boost:: targets detected (in-tree Boost). Using filesystem-first include discovery.")

  set(Boost_FOUND TRUE CACHE INTERNAL "Found in-tree Boost")
  set(BOOST_LIB_DIRS "")

  set(_wt_libs "")
  foreach(_lib program_options thread system filesystem asio chrono numeric serialization spirit array interprocess)
    if(TARGET Boost::${_lib})
      list(APPEND _wt_libs Boost::${_lib})
      string(TOUPPER ${_lib} _ULIB)
      set(BOOST_${_ULIB}_LIB Boost::${_lib})
    endif()
  endforeach()

  set(BOOST_WT_LIBRARIES ${_wt_libs})
  set(BOOST_WTHTTP_LIBRARIES ${_wt_libs})

  if(TARGET Boost::headers)
    list(APPEND BOOST_WT_LIBRARIES Boost::headers)
    list(APPEND BOOST_WTHTTP_LIBRARIES Boost::headers)
  endif()

  set(BOOST_WT_FOUND TRUE)
  set(BOOST_WT_MT_FOUND TRUE)
  set(BOOST_WTHTTP_FOUND TRUE)
  set(BOOST_WTHTTP_MT_FOUND TRUE)

  # --- Function to collect include directories ---
  function(collect_module_includes _moddir _out_var)
    if(NOT _moddir)
      set(${_out_var} "" PARENT_SCOPE)
      return()
    endif()

    set(_raw_list "")

    if(EXISTS "${_moddir}/include" AND IS_DIRECTORY "${_moddir}/include")
      list(APPEND _raw_list "${_moddir}/include")
    endif()

    file(GLOB_RECURSE _found_inc_dirs LIST_DIRECTORIES true "${_moddir}/**/include")
    foreach(_d IN LISTS _found_inc_dirs)
      if(IS_DIRECTORY "${_d}")
        list(APPEND _raw_list "${_d}")
      endif()
    endforeach()

    file(GLOB_RECURSE _found_headers "${_moddir}/**/*.hpp" "${_moddir}/**/*.h" "${_moddir}/**/*.ipp" "${_moddir}/**/*.inl")
    foreach(_hdr IN LISTS _found_headers)
      get_filename_component(_hdr_dir "${_hdr}" DIRECTORY)
      if(_hdr_dir AND IS_DIRECTORY "${_hdr_dir}")
        list(APPEND _raw_list "${_hdr_dir}")
      endif()
    endforeach()

    file(GLOB _children LIST_DIRECTORIES true "${_moddir}/*")
    foreach(_child IN LISTS _children)
      if(IS_DIRECTORY "${_child}")
        if(EXISTS "${_child}/include" AND IS_DIRECTORY "${_child}/include")
          list(APPEND _raw_list "${_child}/include")
        endif()
        file(GLOB _child_hdrs "${_child}/*.{hpp,h,ipp,inl}")
        if(_child_hdrs)
          list(APPEND _raw_list "${_child}")
        endif()
      endif()
    endforeach()

    list(REMOVE_DUPLICATES _raw_list)

    set(_noise_regex "/(doc|docs|example|examples|test|tests|bench|benchmark|benchmarks|\\.github|cmake_subdir_test|\\.git)/")

    set(_final_list "")
    foreach(_p IN LISTS _raw_list)
      if(NOT _p)
        continue()
      endif()
      if("${_p}" MATCHES "\\$<")
        continue()
      endif()

      get_filename_component(_abs_p "${_p}" ABSOLUTE)

      get_filename_component(_basename "${_abs_p}" NAME)
      if(_basename STREQUAL "include")
        if(IS_DIRECTORY "${_abs_p}")
          list(APPEND _final_list "${_abs_p}")
          continue()
        endif()
      endif()

      if("${_abs_p}" MATCHES "${_noise_regex}")
        continue()
      endif()

      file(GLOB _hdrs_direct "${_abs_p}/*.{hpp,h,ipp,inl}")
      if(_hdrs_direct)
        list(APPEND _final_list "${_abs_p}")
        continue()
      endif()

      file(GLOB _immediate_children LIST_DIRECTORIES true "${_abs_p}/*")
      set(_accepted FALSE)
      foreach(_ch IN LISTS _immediate_children)
        if(IS_DIRECTORY "${_ch}")
          file(GLOB _child_hdrs "${_ch}/*.{hpp,h,ipp,inl}")
          if(_child_hdrs)
            set(_accepted TRUE)
            break()
          endif()
        endif()
      endforeach()
      if(_accepted)
        list(APPEND _final_list "${_abs_p}")
      endif()
    endforeach()

    list(REMOVE_DUPLICATES _final_list)

    set(_out_list "")
    foreach(_q IN LISTS _final_list)
      if(IS_DIRECTORY "${_q}")
        get_filename_component(_realq "${_q}" REALPATH)
        if(_realq AND IS_DIRECTORY "${_realq}")
          list(APPEND _out_list "${_realq}")
        endif()
      endif()
    endforeach()

    list(REMOVE_DUPLICATES _out_list)
    set(${_out_var} "${_out_list}" PARENT_SCOPE)
  endfunction()

  # Scan modules
  set(_collected_includes_raw "")
  foreach(_mod IN LISTS Boost_COMPONENTS)
    string(STRIP "${_mod}" _mod_trim)
    if(NOT _mod_trim)
      continue()
    endif()

    set(_moddir "${CMAKE_SOURCE_DIR}/boost/libs/${_mod_trim}")
    if(NOT EXISTS "${_moddir}")
      set(_moddir_alt "${CMAKE_CURRENT_SOURCE_DIR}/boost/libs/${_mod_trim}")
      if(EXISTS "${_moddir_alt}")
        set(_moddir "${_moddir_alt}")
      endif()
    endif()

    if(NOT EXISTS "${_moddir}")
      message(STATUS "WtFindBoost: module not present, skipping: ${_mod_trim}")
      continue()
    endif()

    collect_module_includes("${_moddir}" _module_incs)
    foreach(_inc IN LISTS _module_incs)
      list(APPEND _collected_includes_raw "${_inc}")
    endforeach()
  endforeach()

  list(REMOVE_DUPLICATES _collected_includes_raw)

  set(BOOST_INCLUDE_DIRS ${_collected_includes_raw} CACHE INTERNAL "Collected in-tree Boost include dirs (filtered)")

  # Attach BOOST_INCLUDE_DIRS to Boost targets — resolve aliases to real targets when possible
  foreach(_t IN LISTS BOOST_WT_LIBRARIES)
    if(NOT _t OR NOT TARGET ${_t})
      continue()
    endif()

    get_target_property(_aliased ${_t} ALIASED_TARGET)
    if(DEFINED _aliased AND NOT "${_aliased}" STREQUAL "NOTFOUND")
      set(_real_target ${_aliased})
    else()
      set(_real_target ${_t})
    endif()

    if(TARGET ${_real_target})
      get_target_property(_real_aliased ${_real_target} ALIASED_TARGET)
      if(DEFINED _real_aliased AND NOT "${_real_aliased}" STREQUAL "NOTFOUND")
        message(STATUS "WtFindBoost: resolved target ${_t} -> ${_real_target} but it is still an ALIAS; skipping attach")
        continue()
      endif()

      get_target_property(_t_type ${_real_target} TYPE)
      if(DEFINED _t_type AND NOT "${_t_type}" STREQUAL "UNKNOWN")
        message(VERBOSE "WtFindBoost: attaching BOOST_INCLUDE_DIRS to resolved target ${_real_target} (from ${_t}) type=${_t_type}")
        target_include_directories(${_real_target} INTERFACE ${BOOST_INCLUDE_DIRS})
      else()
        message(STATUS "WtFindBoost: not attaching include dirs to ${_real_target} (unknown type)")
      endif()
    endif()
  endforeach()

  foreach(_inc IN LISTS BOOST_INCLUDE_DIRS)
    message(STATUS "WtFindBoost: adding include dir => ${_inc}")
  endforeach()
endif()

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ PROGRESS FIVE- BEST FOUR @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

