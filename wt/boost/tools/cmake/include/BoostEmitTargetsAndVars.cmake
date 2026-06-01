# BoostEmitTargetsAndVars.cmake
# Patched: robust generator that emits BoostTargets.cmake + BoostConfig.cmake
# with safe handling for ALIAS targets (resolves aliased target before setting properties).
cmake_minimum_required(VERSION 3.14)

# Candidate library roots (adjust if your build uses different layout)
set(_candidate_lib_roots
  "${CMAKE_BINARY_DIR}/boost/libs"
  "${CMAKE_BINARY_DIR}/stage/lib"
  "${CMAKE_BINARY_DIR}/lib"
  "${CMAKE_BINARY_DIR}"
)

# Candidate include roots (prioritize per-lib under source tree)
set(_candidate_include_roots
  "${CMAKE_SOURCE_DIR}/boost/libs"
  "${CMAKE_SOURCE_DIR}/cmakesource/boost/libs/include"
  "${CMAKE_SOURCE_DIR}/boost"
)

# Full list of boost libs to emit targets for (extend as needed)
set(_BOOST_LIB_LIST
  accumulators algorithm align any array asio assert assign atomic beast bimap bind bloom
  callable_traits charconv chrono circular_buffer cobalt compat compute concept_check config
  container container_hash context contract conversion convert core coroutine coroutine2 crc
  date_time describe detail dll dynamic_bitset endian exception fiber filesystem flyweight
  foreach format function function_types functional fusion geometry gil graph graph_parallel
  hana hash2 headers heap histogram hof icl integer interprocess intrusive io iostreams iterator
  json lambda lambda2 leaf lexical_cast locale lockfree log logic math metaparse move mp11 mpi
  mpl mqtt5 msm multi_array multi_index multiprecision mysql nowide openmethod optional outcome
  parameter parser pfr phoenix poly_collection polygon pool predef preprocessor process
  program_options property_map property_map_parallel property_tree proto ptr_container python qvm
  random range ratio rational redis regex safe_numerics scope scope_exit serialization signals2
  smart_ptr sort spirit stacktrace statechart static_assert static_string stl_interfaces system test
  thread throw_exception timer tokenizer tti tuple type_erasure type_index type_traits typeof units
  unordered url utility uuid variant variant2 vmd wave winapi xpressive yap
)

# Output locations for generated config
set(BOOST_CONFIG_DIR "${CMAKE_BINARY_DIR}/boost")
file(MAKE_DIRECTORY "${BOOST_CONFIG_DIR}")
set(_targets_file "${BOOST_CONFIG_DIR}/BoostTargets.cmake")
set(_config_file  "${BOOST_CONFIG_DIR}/BoostConfig.cmake")
set(_configver_file "${BOOST_CONFIG_DIR}/BoostConfigVersion.cmake")

# ------------------- Helper functions -------------------

# Convert to CMake-style path (returns empty string if input empty)
function(_to_cmake_path in out)
  if(NOT "${in}" STREQUAL "")
    file(TO_CMAKE_PATH "${in}" _tmp)
    set(${out} "${_tmp}" PARENT_SCOPE)
  else()
    set(${out} "" PARENT_SCOPE)
  endif()
endfunction()

# Sanitize raw library name to safe token for target names
function(_sanitize_name raw out)
  if("${raw}" STREQUAL "")
    set(${out} "" PARENT_SCOPE)
    return()
  endif()
  string(REGEX REPLACE ".*/" "" _b "${raw}")                # drop directories
  string(REGEX REPLACE "^libboost_" "" _b "${_b}")
  string(REGEX REPLACE "^boost_" "" _b "${_b}")
  string(REGEX REPLACE "[-\\.].*$" "" _b "${_b}")          # strip -mt and .so.version
  string(REGEX REPLACE "[/\\\\:]" "_" _b "${_b}")          # replace slashes/colons
  string(REPLACE "-" "_" _b "${_b}")
  set(${out} "${_b}" PARENT_SCOPE)
endfunction()

# Safe set_target_properties — resolves ALIAS targets to aliased target name first
function(_safe_set_target_property tgt prop value)
  if("${tgt}" STREQUAL "") return() endif()
  get_target_property(_tgt_type "${tgt}" TYPE)
  if("${_tgt_type}" STREQUAL "ALIAS")
    get_target_property(_aliased "${tgt}" ALIASED_TARGET)
    if(NOT "${_aliased}" STREQUAL "")
      set_target_properties("${_aliased}" PROPERTIES ${prop} "${value}")
    else()
      message(WARNING "BoostEmitTargetsAndVars: alias ${tgt} found but ALIASED_TARGET unknown; skipping set_property ${prop}")
    endif()
  else()
    set_target_properties("${tgt}" PROPERTIES ${prop} "${value}")
  endif()
endfunction()

# Safe append to INTERFACE_LINK_LIBRARIES — handles ALIAS targets too
function(_safe_append_interface_link tgt value)
  if("${tgt}" STREQUAL "") return() endif()
  get_target_property(_tgt_type2 "${tgt}" TYPE)
  if("${_tgt_type2}" STREQUAL "ALIAS")
    get_target_property(_aliased2 "${tgt}" ALIASED_TARGET)
    if(NOT "${_aliased2}" STREQUAL "")
      set_property(TARGET "${_aliased2}" APPEND PROPERTY INTERFACE_LINK_LIBRARIES "${value}")
    else()
      message(WARNING "BoostEmitTargetsAndVars: alias ${tgt} found but ALIASED_TARGET unknown; skipping append INTERFACE_LINK_LIBRARIES")
    endif()
  else()
    set_property(TARGET "${tgt}" APPEND PROPERTY INTERFACE_LINK_LIBRARIES "${value}")
  endif()
endfunction()

# ------------------- Discovery helpers -------------------

# Discover per-lib include directory (strict per-lib under source tree preferred)
function(_discover_include lib out)
  set(_found "")
  set(_perlib "${CMAKE_SOURCE_DIR}/boost/libs/${lib}/include")
  if(EXISTS "${_perlib}")
    set(_found "${_perlib}")
    set(${out} "${_found}" PARENT_SCOPE)
    return()
  endif()

  foreach(_root IN LISTS _candidate_include_roots)
    if("${_root}" STREQUAL "") continue() endif()
    set(_cand "${_root}/${lib}/include")
    if(EXISTS "${_cand}")
      set(_found "${_cand}")
      break()
    endif()
  endforeach()

  set(${out} "${_found}" PARENT_SCOPE)
endfunction()

# Discover artifact — prioritize ${CMAKE_BINARY_DIR}/boost/libs/<lib>/libboost_<lib>*
function(_discover_artifact lib out)
  set(_found "")

  set(_per_lib_expected "${CMAKE_BINARY_DIR}/boost/libs/${lib}")
  if(EXISTS "${_per_lib_expected}")
    file(GLOB _per_c RELATIVE "${_per_lib_expected}"
      "${_per_lib_expected}/libboost_${lib}*"
      "${_per_lib_expected}/boost_${lib}*"
      "${_per_lib_expected}/${lib}*"
    )
    if(_per_c)
      list(SORT _per_c)
      list(GET _per_c 0 _p0)
      if(NOT "${_p0}" STREQUAL "")
        set(_found "${_per_lib_expected}/${_p0}")
        set(${out} "${_found}" PARENT_SCOPE)
        return()
      endif()
    endif()
  endif()

  foreach(_root IN LISTS _candidate_lib_roots)
    if("${_root}" STREQUAL "") continue() endif()
    file(GLOB _c2 RELATIVE "${_root}"
      "${_root}/libboost_${lib}*"
      "${_root}/boost_${lib}*"
      "${_root}/${lib}*"
    )
    if(_c2)
      list(SORT _c2)
      list(GET _c2 0 _f2)
      if(NOT "${_f2}" STREQUAL "")
        set(_found "${_root}/${_f2}")
        break()
      endif()
    endif()
  endforeach()

  if("${_found}" STREQUAL "")
    foreach(_root IN LISTS _candidate_lib_roots)
      if("${_root}" STREQUAL "") continue() endif()
      file(GLOB _c3 RELATIVE "${_root}" "${_root}/*boost*${lib}*" "${_root}/*${lib}*")
      if(_c3)
        list(SORT _c3)
        list(GET _c3 0 _f3)
        if(NOT "${_f3}" STREQUAL "")
          set(_found "${_root}/${_f3}")
          break()
        endif()
      endif()
    endforeach()
  endif()

  set(${out} "${_found}" PARENT_SCOPE)
endfunction()

# ------------------- Generate targets and config -------------------

set(_include_dirs "")
set(_lib_dirs "")
set(_component_map "")   # entries like "filesystem=/path/to/lib"

# Start targets file (header)
file(WRITE "${_targets_file}" "# Auto-generated BoostTargets.cmake\ncmake_minimum_required(VERSION 3.14)\n\n")

foreach(_lib IN LISTS _BOOST_LIB_LIST)
  if("${_lib}" STREQUAL "") continue() endif()

  # Discover include and artifact
  _discover_include("${_lib}" _inc)
  _discover_artifact("${_lib}" _art)

  if(NOT "${_inc}" STREQUAL "") list(APPEND _include_dirs "${_inc}") endif()
  if(NOT "${_art}" STREQUAL "")
    get_filename_component(_art_dir "${_art}" DIRECTORY)
    if(NOT "${_art_dir}" STREQUAL "") list(APPEND _lib_dirs "${_art_dir}") endif()
    set(_component_map "${_component_map};${_lib}=${_art}")
  endif()

  _sanitize_name("${_lib}" _san)
  if("${_san}" STREQUAL "")
    message(STATUS "BoostEmitTargetsAndVars: skipping invalid lib name '${_lib}'")
    continue()
  endif()

  set(_tgt "Boost::${_san}")

  # Ensure target exists (live)
  if(NOT TARGET "${_tgt}")
    add_library("${_tgt}" INTERFACE IMPORTED GLOBAL)
  endif()

  # Set include dirs safely
  if(NOT "${_inc}" STREQUAL "")
    _to_cmake_path("${_inc}" _inc_p)
    if(NOT "${_inc_p}" STREQUAL "")
      _safe_set_target_property("${_tgt}" INTERFACE_INCLUDE_DIRECTORIES "${_inc_p}")
    endif()
  endif()

  # Attach artifact if present (safe)
  if(NOT "${_art}" STREQUAL "")
    _to_cmake_path("${_art}" _art_p)
    if(NOT "${_art_p}" STREQUAL "")
      _safe_set_target_property("${_tgt}" IMPORTED_LOCATION "${_art_p}")
      _safe_append_interface_link("${_tgt}" "${_art_p}")
    endif()
  endif()

  # Append guarded creation code to targets file so find_package consumers create same targets
  file(APPEND "${_targets_file}" "if(NOT TARGET ${_tgt})\n")
  file(APPEND "${_targets_file}" "  add_library(${_tgt} INTERFACE IMPORTED GLOBAL)\n")
  if(NOT "${_inc}" STREQUAL "")
    _to_cmake_path("${_inc}" _inc_p2)
    file(APPEND "${_targets_file}" "  set_target_properties(${_tgt} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES \"${_inc_p2}\")\n")
  endif()
  if(NOT "${_art}" STREQUAL "")
    _to_cmake_path("${_art}" _art_p2)
    file(APPEND "${_targets_file}" "  set_target_properties(${_tgt} PROPERTIES IMPORTED_LOCATION \"${_art_p2}\")\n")
    file(APPEND "${_targets_file}" "  set_property(TARGET ${_tgt} APPEND PROPERTY INTERFACE_LINK_LIBRARIES \"${_art_p2}\")\n")
  endif()
  file(APPEND "${_targets_file}" "endif()\n\n")
endforeach()

# Deduplicate include/lib dirs
list(REMOVE_DUPLICATES _include_dirs)
list(REMOVE_DUPLICATES _lib_dirs)

# Write BoostConfig.cmake
file(WRITE "${_config_file}" "# Auto-generated BoostConfig.cmake\ncmake_minimum_required(VERSION 3.14)\nset(Boost_FOUND TRUE)\n")

# Build include and lib dir strings
set(_inc_cm "")
foreach(_d IN LISTS _include_dirs)
  _to_cmake_path("${_d}" _dc)
  if(NOT "${_dc}" STREQUAL "")
    if("${_inc_cm}" STREQUAL "")
      set(_inc_cm "${_dc}")
    else()
      set(_inc_cm "${_inc_cm};${_dc}")
    endif()
  endif()
endforeach()

set(_lib_cm "")
foreach(_d IN LISTS _lib_dirs)
  _to_cmake_path("${_d}" _dc2)
  if(NOT "${_dc2}" STREQUAL "")
    if("${_lib_cm}" STREQUAL "")
      set(_lib_cm "${_dc2}")
    else()
      set(_lib_cm "${_lib_cm};${_dc2}")
    endif()
  endif()
endforeach()

file(APPEND "${_config_file}" "set(Boost_INCLUDE_DIRS \"${_inc_cm}\")\n")
file(APPEND "${_config_file}" "set(Boost_LIBRARY_DIRS \"${_lib_cm}\")\n")

# Helper to extract component artifacts from _component_map
function(_component_value comp out)
  set(_res "")
  if(NOT "${_component_map}" STREQUAL "")
    foreach(_map_item IN LISTS _component_map)
      string(FIND "${_map_item}" "=" _eq)
      if(_eq GREATER -1)
        string(SUBSTRING "${_map_item}" 0 ${_eq} _k)
        math(EXPR _vstart "${_eq} + 1")
        string(SUBSTRING "${_map_item}" ${_vstart} -1 _v)
        if("${_k}" STREQUAL "${comp}")
          set(_res "${_v}")
          break()
        endif()
      endif()
    endforeach()
  endif()
  set(${out} "${_res}" PARENT_SCOPE)
endfunction()

_component_value("filesystem" _fs)
_component_value("program_options" _po)
_component_value("system" _sys)
_component_value("thread" _thr)

_to_cmake_path("${_fs}" _fs_c)
_to_cmake_path("${_po}" _po_c)
_to_cmake_path("${_sys}" _sys_c)
_to_cmake_path("${_thr}" _thr_c)

file(APPEND "${_config_file}" "set(Boost_FILESYSTEM_LIBRARY \"${_fs_c}\")\n")
file(APPEND "${_config_file}" "set(Boost_PROGRAM_OPTIONS_LIBRARY \"${_po_c}\")\n")
file(APPEND "${_config_file}" "set(Boost_SYSTEM_LIBRARY \"${_sys_c}\")\n")
file(APPEND "${_config_file}" "set(Boost_THREAD_LIBRARY \"${_thr_c}\")\n")

# Wt legacy variables
file(APPEND "${_config_file}" "set(BOOST_INCLUDE_DIRS \"${_inc_cm}\")\n")
file(APPEND "${_config_file}" "set(BOOST_LIB_DIRS \"${_lib_cm}\")\n")
file(APPEND "${_config_file}" "set(BOOST_FS_LIB \"${_fs_c}\")\n")
file(APPEND "${_config_file}" "set(BOOST_PO_LIB \"${_po_c}\")\n")
file(APPEND "${_config_file}" "set(BOOST_SYSTEM_LIB \"${_sys_c}\")\n")
file(APPEND "${_config_file}" "set(BOOST_THREAD_LIB \"${_thr_c}\")\n")

# Compose BOOST_WT_LIBRARIES and BOOST_WTHTTP_LIBRARIES
set(_wtlibs "")
if(NOT "${_thr_c}" STREQUAL "") set(_wtlibs "${_thr_c}") endif()
if(NOT "${_sys_c}" STREQUAL "")
  if("${_wtlibs}" STREQUAL "") set(_wtlibs "${_sys_c}") else set(_wtlibs "${_wtlibs};${_sys_c}") endif()
endif()
if(NOT "${_fs_c}" STREQUAL "")
  if("${_wtlibs}" STREQUAL "") set(_wtlibs "${_fs_c}") else set(_wtlibs "${_wtlibs};${_fs_c}") endif()
endif()

set(_wthttplibs "")
if(NOT "${_thr_c}" STREQUAL "") set(_wthttplibs "${_thr_c}") endif()
if(NOT "${_po_c}" STREQUAL "")
  if("${_wthttplibs}" STREQUAL "") set(_wthttplibs "${_po_c}") else set(_wthttplibs "${_wthttplibs};${_po_c}") endif()
endif()
if(NOT "${_sys_c}" STREQUAL "")
  if("${_wthttplibs}" STREQUAL "") set(_wthttplibs "${_sys_c}") else set(_wthttplibs "${_wthttplibs};${_sys_c}") endif()
endif()
if(NOT "${_fs_c}" STREQUAL "")
  if("${_wthttplibs}" STREQUAL "") set(_wthttplibs "${_fs_c}") else set(_wthttplibs "${_wthttplibs};${_fs_c}") endif()
endif()

file(APPEND "${_config_file}" "set(BOOST_WT_LIBRARIES \"${_wtlibs}\")\n")
file(APPEND "${_config_file}" "set(BOOST_WTHTTP_LIBRARIES \"${_wthttplibs}\")\n")
file(APPEND "${_config_file}" "set(BOOST_SUPPORT_LIBRARIES \"\")\n")

# Ensure find_package consumers get targets
file(APPEND "${_config_file}" "include(\"\${CMAKE_CURRENT_LIST_DIR}/BoostTargets.cmake\")\n")

# Write version file
file(WRITE "${_configver_file}" "set(PACKAGE_VERSION \"1.0.0\")\n")

# Make discoverable in same configure
list(APPEND CMAKE_PREFIX_PATH "${BOOST_CONFIG_DIR}")

message(STATUS "BoostEmitTargetsAndVars: wrote ${_targets_file}")
message(STATUS "BoostEmitTargetsAndVars: wrote ${_config_file}")
message(STATUS "BoostEmitTargetsAndVars: appended ${BOOST_CONFIG_DIR} to CMAKE_PREFIX_PATH")

#[[ BoostEmitTargetsAndVars.cmake
# Robust generator that emits BoostTargets.cmake and BoostConfig.cmake
# - Prioritizes per-lib artifacts in: ${CMAKE_BINARY_DIR}/boost/libs/<lib>/libboost_<lib>*
# - Discovers per-lib include dirs at: ${CMAKE_SOURCE_DIR}/boost/libs/<lib>/include
# - Creates guarded Boost::<lib> IMPORTED targets (sanitized names)
# - Writes BoostTargets.cmake, BoostConfig.cmake, BoostConfigVersion.cmake in ${CMAKE_BINARY_DIR}/boost
# - Sets Boost_* and BOOST_* variables for Wt compatibility
#
# Include this AFTER your Boost build / install steps so the artifacts exist.

cmake_minimum_required(VERSION 3.14)

# --- configuration: candidate search roots and include roots (tune if needed) ---
set(_candidate_lib_roots
  "${Boost_BINARY_DIR}"                # optional variable set by Boost fragments
  "${Boost_INSTALL_DIR}"               # optional variable set by BoostInstall
  "${CMAKE_BINARY_DIR}/stage/lib"      # b2 staging
  "${CMAKE_BINARY_DIR}/lib"
  "${CMAKE_BINARY_DIR}/boost/libs"     # common in your layout
  "${CMAKE_BINARY_DIR}/boost"
  "${CMAKE_BINARY_DIR}"
)

set(_candidate_include_roots
  "${Boost_SOURCE_DIR}"                                  # optional Boost source variable
  "${CMAKE_SOURCE_DIR}/boost"                            # top-level source boost dir
  "${CMAKE_SOURCE_DIR}/cmakesource/boost/libs/include"   # your earlier mention
  "${CMAKE_SOURCE_DIR}/boost/libs"                       # per-lib include directories under here
)

# canonical list of Boost libs to emit (your list)
set(_BOOST_LIB_LIST
  accumulators algorithm align any array asio assert assign atomic beast bimap bind bloom
  callable_traits charconv chrono circular_buffer cobalt compat compute concept_check config
  container container_hash context contract conversion convert core coroutine coroutine2 crc
  date_time describe detail dll dynamic_bitset endian exception fiber filesystem flyweight
  foreach format function function_types functional fusion geometry gil graph graph_parallel
  hana hash2 headers heap histogram hof icl integer interprocess intrusive io iostreams iterator
  json lambda lambda2 leaf lexical_cast locale lockfree log logic math metaparse move mp11 mpi
  mpl mqtt5 msm multi_array multi_index multiprecision mysql nowide openmethod optional outcome
  parameter parser pfr phoenix poly_collection polygon pool predef preprocessor process
  program_options property_map property_map_parallel property_tree proto ptr_container python qvm
  random range ratio rational redis regex safe_numerics scope scope_exit serialization signals2
  smart_ptr sort spirit stacktrace statechart static_assert static_string stl_interfaces system test
  thread throw_exception timer tokenizer tti tuple type_erasure type_index type_traits typeof units
  unordered url utility uuid variant variant2 vmd wave winapi xpressive yap
)

# output dir for generated config
set(BOOST_CONFIG_DIR "${CMAKE_BINARY_DIR}/boost")
file(MAKE_DIRECTORY "${BOOST_CONFIG_DIR}")

# path of the targets file we will write
set(_targets_file "${BOOST_CONFIG_DIR}/BoostTargets.cmake")

# ---------- helper functions ----------

# convert to cmake style path; return empty string if input empty
function(_to_cmake_path in out)
  if(NOT in OR in STREQUAL "")
    # fall through, convert
  endif()
  if(in AND NOT in STREQUAL "")
    file(TO_CMAKE_PATH "${in}" _tmp)
    set(${out} "${_tmp}" PARENT_SCOPE)
  else()
    set(${out} "" PARENT_SCOPE)
  endif()
endfunction()

# discover include dir: prefer per-lib include at ${CMAKE_SOURCE_DIR}/boost/libs/<lib>/include
function(_discover_include lib out)
  set(_found "")
  # first try strictly the per-lib include you requested
  set(_perlib_include "${CMAKE_SOURCE_DIR}/boost/libs/${lib}/include")
  if(EXISTS "${_perlib_include}")
    set(_found "${_perlib_include}")
    set(${out} "${_found}" PARENT_SCOPE)
    return()
  endif()

  # fallback: try configured candidate roots
  foreach(_root IN LISTS _candidate_include_roots)
    if(NOT _root OR _root STREQUAL "")
      continue()
    endif()
    set(_cand1 "${_root}/${lib}/include")
    if(EXISTS "${_cand1}")
      set(_found "${_cand1}")
      break()
    endif()
    if(EXISTS "${_root}/boost")
      set(_found "${_root}")
      break()
    endif()
  endforeach()

  set(${out} "${_found}" PARENT_SCOPE)
endfunction()

# discover artifact, prioritizing per-lib folder at ${CMAKE_BINARY_DIR}/boost/libs/<lib>/
function(_discover_artifact lib out)
  set(_found "")

  # 1) check exact per-lib expected folder first (your specified layout)
  set(_per_lib_expected "${CMAKE_BINARY_DIR}/boost/libs/${lib}")
  if(EXISTS "${_per_lib_expected}")
    file(GLOB _perlib_cands RELATIVE "${_per_lib_expected}"
      "${_per_lib_expected}/libboost_${lib}*"
      "${_per_lib_expected}/boost_${lib}*"
      "${_per_lib_expected}/${lib}*"
    )
    if(_perlib_cands)
      list(SORT _perlib_cands)
      list(GET _perlib_cands 0 _p0)
      if(_p0)
        set(_found "${_per_lib_expected}/${_p0}")
        set(${out} "${_found}" PARENT_SCOPE)
        return()
      endif()
    endif()
  endif()

  # 2) fallback: scan candidate lib roots (per-lib subdir then root patterns)
  foreach(_root IN LISTS _candidate_lib_roots)
    if(NOT _root OR _root STREQUAL "")
      continue()
    endif()

    set(_perdir "${_root}/${lib}")
    if(EXISTS "${_perdir}")
      file(GLOB _cand1 RELATIVE "${_perdir}"
        "${_perdir}/libboost_${lib}*"
        "${_perdir}/boost_${lib}*"
        "${_perdir}/${lib}*"
      )
      if(_cand1)
        list(SORT _cand1)
        list(GET _cand1 0 _n)
        if(_n)
          set(_found "${_perdir}/${_n}")
          break()
        endif()
      endif()
    endif()

    file(GLOB _cand2 RELATIVE "${_root}"
      "${_root}/libboost_${lib}*"
      "${_root}/boost_${lib}*"
      "${_root}/${lib}*"
    )
    if(_cand2)
      list(SORT _cand2)
      list(GET _cand2 0 _n2)
      if(_n2)
        set(_found "${_root}/${_n2}")
        break()
      endif()
    endif()
  endforeach()

  # 3) last relaxed fallback (scan for names containing lib name)
  if(NOT _found)
    foreach(_root IN LISTS _candidate_lib_roots)
      if(NOT _root OR _root STREQUAL "")
        continue()
      endif()
      file(GLOB _cand3 RELATIVE "${_root}"
        "${_root}/*boost*${lib}*"
        "${_root}/*${lib}*"
      )
      if(_cand3)
        list(SORT _cand3)
        list(GET _cand3 0 _n3)
        if(_n3)
          set(_found "${_root}/${_n3}")
          break()
        endif()
      endif()
    endforeach()
  endif()

  set(${out} "${_found}" PARENT_SCOPE)
endfunction()

# convert a filename or basename to canonical boost component name (defensive)
function(_sanitize_libname raw out)
  if(NOT raw)
    set(${out} "" PARENT_SCOPE)
    return()
  endif()
  # remove any path separators, leading libboost_ or boost_, and suffixes after dash/dot
  string(REGEX REPLACE ".*/" "" _b "${raw}")                # strip directories
  string(REGEX REPLACE "^libboost_" "" _b "${_b}")
  string(REGEX REPLACE "^boost_" "" _b "${_b}")
  string(REGEX REPLACE "[-\\.].*$" "" _b "${_b}")          # cut "-mt", ".so.1.75" etc.
  string(REGEX REPLACE "[/\\\\]" "_" _b "${_b}")           # replace any stray slashes
  string(REPLACE "-" "_" _b "${_b}")
  set(${out} "${_b}" PARENT_SCOPE)
endfunction()

# ---------- discovery pass ----------

set(_emit_entries "")     # each entry: "<lib>;<inc>;<artifact>"
set(_include_dirs_list "")
set(_lib_dirs_list "")
set(_component_map "")    # semicolon-separated entries "component=path"

foreach(_lib IN LISTS _BOOST_LIB_LIST)
  # keep original name for discovery then sanitize for target name
  set(_orig "${_lib}")
  _discover_include("${_orig}" _inc)
  _discover_artifact("${_orig}" _art)

  if(_inc)
    list(APPEND _include_dirs_list "${_inc}")
  endif()

  if(_art)
    get_filename_component(_art_dir "${_art}" DIRECTORY)
    if(_art_dir)
      list(APPEND _lib_dirs_list "${_art_dir}")
    endif()
    # store mapping
    set(_component_map "${_component_map};${_orig}=${_art}")
  endif()

  # Append the string entry; we will parse it safely later
  list(APPEND _emit_entries "${_orig};${_inc};${_art}")
endforeach()

# dedupe
list(REMOVE_DUPLICATES _include_dirs_list)
list(REMOVE_DUPLICATES _lib_dirs_list)

# ---------- write an initially empty targets file (we will also create targets live) ----------
file(WRITE "${_targets_file}" "# Auto-generated BoostTargets.cmake (generated by BoostEmitTargetsAndVars.cmake)\n")
file(APPEND "${_targets_file}" "cmake_minimum_required(VERSION 3.14)\n\n")

# ---------- create/import targets LIVE and append guarded declarations to the targets file ----------
foreach(_entry IN LISTS _emit_entries)
  # _entry is a semicolon-separated string; convert to real list 'parts'
  set(_parts ${_entry})   # now _parts is a CMake list (safe)
  list(LENGTH _parts _parts_len)
  if(_parts_len LESS 1)
    message(STATUS "BoostEmitTargetsAndVars: skipping malformed entry (empty): '${_entry}'")
    continue()
  endif()

  # extract safely
  list(GET _parts 0 _lib_orig "")       # original lib name (e.g. "thread")
  if(_parts_len GREATER 1)
    list(GET _parts 1 _inc "")
  else()
    set(_inc "")
  endif()
  if(_parts_len GREATER 2)
    list(GET _parts 2 _art "")
  else()
    set(_art "")
  endif()

  if(NOT _lib_orig OR _lib_orig STREQUAL "")
    message(STATUS "BoostEmitTargetsAndVars: skipping entry with empty lib name (entry='${_entry}')")
    continue()
  endif()

  # sanitize lib name for a target (remove slashes, dots, dashes; fall back to safe token)
  _sanitize_libname("${_lib_orig}" _lib_sanitized)
  if(NOT _lib_sanitized OR _lib_sanitized STREQUAL "")
    message(STATUS "BoostEmitTargetsAndVars: sanitized lib name empty for original='${_lib_orig}', skipping")
    continue()
  endif()

  # guard: do not create target names that contain colon or path-like tokens
  string(FIND "${_lib_sanitized}" ":" _has_colon)
  if(NOT _has_colon EQUAL -1)
    string(REGEX REPLACE ":" "_" _lib_sanitized "${_lib_sanitized}")
  endif()

  # Live-create a guarded imported target so other in-tree code can use it immediately
  if(NOT TARGET Boost::${_lib_sanitized})
    add_library(Boost::${_lib_sanitized} INTERFACE IMPORTED GLOBAL)
  endif()

  # set include dir (if discovered)
  if(_inc AND NOT _inc STREQUAL "")
    _to_cmake_path("${_inc}" _inc_cmake)
    if(_inc_cmake AND NOT _inc_cmake STREQUAL "")
      set_target_properties(Boost::${_lib_sanitized} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${_inc_cmake}")
    endif()
  endif()

  # set imported artifact if discovered
  if(_art AND NOT _art STREQUAL "")
    _to_cmake_path("${_art}" _art_cmake)
    if(_art_cmake AND NOT _art_cmake STREQUAL "")
      set_target_properties(Boost::${_lib_sanitized} PROPERTIES IMPORTED_LOCATION "${_art_cmake}")
      set_property(TARGET Boost::${_lib_sanitized} APPEND PROPERTY INTERFACE_LINK_LIBRARIES "${_art_cmake}")
    endif()
  endif()

  # Append a guarded creation block to the targets file for external find_package consumers
  # Use the sanitized name to avoid invalid target names.
  file(APPEND "${_targets_file}" "if(NOT TARGET Boost::${_lib_sanitized})\n")
  file(APPEND "${_targets_file}" "  add_library(Boost::${_lib_sanitized} INTERFACE IMPORTED GLOBAL)\n")
  if(_inc AND NOT _inc STREQUAL "")
    _to_cmake_path("${_inc}" _inc_cmake2)
    file(APPEND "${_targets_file}" "  set_target_properties(Boost::${_lib_sanitized} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES \"${_inc_cmake2}\")\n")
  endif()
  if(_art AND NOT _art STREQUAL "")
    _to_cmake_path("${_art}" _art_cmake2)
    file(APPEND "${_targets_file}" "  set_target_properties(Boost::${_lib_sanitized} PROPERTIES IMPORTED_LOCATION \"${_art_cmake2}\")\n")
    file(APPEND "${_targets_file}" "  set_property(TARGET Boost::${_lib_sanitized} APPEND PROPERTY INTERFACE_LINK_LIBRARIES \"${_art_cmake2}\")\n")
  endif()
  file(APPEND "${_targets_file}" "endif()\n\n")
endforeach()

message(STATUS "BoostEmitTargetsAndVars: wrote ${_targets_file}")

# ---------- Write BoostConfig.cmake and variables for both find_package and WtFindBoost ----------
set(_config_file "${BOOST_CONFIG_DIR}/BoostConfig.cmake")
file(WRITE "${_config_file}" "# Auto-generated BoostConfig.cmake\n")
file(APPEND "${_config_file}" "cmake_minimum_required(VERSION 3.14)\n")
file(APPEND "${_config_file}" "set(Boost_FOUND TRUE)\n")

# prepare include list string
set(_inc_list_cm "")
if(_include_dirs_list)
  foreach(_d IN LISTS _include_dirs_list)
    _to_cmake_path("${_d}" _dc)
    if(_inc_list_cm STREQUAL "")
      set(_inc_list_cm "${_dc}")
    else()
      set(_inc_list_cm "${_inc_list_cm};${_dc}")
    endif()
  endforeach()
endif()

# prepare lib dir list string
set(_lib_list_cm "")
if(_lib_dirs_list)
  foreach(_d IN LISTS _lib_dirs_list)
    _to_cmake_path("${_d}" _dc2)
    if(_lib_list_cm STREQUAL "")
      set(_lib_list_cm "${_dc2}")
    else()
      set(_lib_list_cm "${_lib_list_cm};${_dc2}")
    endif()
  endforeach()
endif()

file(APPEND "${_config_file}" "set(Boost_INCLUDE_DIRS \"${_inc_list_cm}\")\n")
file(APPEND "${_config_file}" "set(Boost_LIBRARY_DIRS \"${_lib_list_cm}\")\n")

# helper: read mapping from _component_map (which is semicolon-separated "lib=path" entries)
function(_component_value comp out)
  set(_res "")
  if(_component_map)
    foreach(_map_item IN LISTS _component_map)
      # split "k=v"
      string(FIND "${_map_item}" "=" _eq)
      if(_eq GREATER -1)
        string(SUBSTRING "${_map_item}" 0 ${_eq} _k)
        math(EXPR _vstart "${_eq} + 1")
        string(SUBSTRING "${_map_item}" ${_vstart} -1 _v)
        if(_k STREQUAL "${comp}")
          set(_res "${_v}")
          break()
        endif()
      endif()
    endforeach()
  endif()
  set(${out} "${_res}" PARENT_SCOPE)
endfunction()

_component_value("filesystem" _fs)
_component_value("program_options" _po)
_component_value("system" _sys)
_component_value("thread" _thr)

# convert to cmake path variables
_to_cmake_path("${_fs}" _fs_c)
_to_cmake_path("${_po}" _po_c)
_to_cmake_path("${_sys}" _sys_c)
_to_cmake_path("${_thr}" _thr_c)

file(APPEND "${_config_file}" "set(Boost_FILESYSTEM_LIBRARY \"${_fs_c}\")\n")
file(APPEND "${_config_file}" "set(Boost_PROGRAM_OPTIONS_LIBRARY \"${_po_c}\")\n")
file(APPEND "${_config_file}" "set(Boost_SYSTEM_LIBRARY \"${_sys_c}\")\n")
file(APPEND "${_config_file}" "set(Boost_THREAD_LIBRARY \"${_thr_c}\")\n")

# Wt legacy variables
file(APPEND "${_config_file}" "set(BOOST_INCLUDE_DIRS \"${_inc_list_cm}\")\n")
file(APPEND "${_config_file}" "set(BOOST_LIB_DIRS \"${_lib_list_cm}\")\n")
file(APPEND "${_config_file}" "set(BOOST_FS_LIB \"${_fs_c}\")\n")
file(APPEND "${_config_file}" "set(BOOST_PO_LIB \"${_po_c}\")\n")
file(APPEND "${_config_file}" "set(BOOST_SYSTEM_LIB \"${_sys_c}\")\n")
file(APPEND "${_config_file}" "set(BOOST_THREAD_LIB \"${_thr_c}\")\n")

# compose Wt-specific lists:
set(_wtlibs "")
if(NOT "${_thr_c}" STREQUAL "")
  set(_wtlibs "${_thr_c}")
endif()
if(NOT "${_sys_c}" STREQUAL "")
  if(_wtlibs STREQUAL "")
    set(_wtlibs "${_sys_c}")
  else()
    set(_wtlibs "${_wtlibs};${_sys_c}")
  endif()
endif()
if(NOT "${_fs_c}" STREQUAL "")
  if(_wtlibs STREQUAL "")
    set(_wtlibs "${_fs_c}")
  else()
    set(_wtlibs "${_wtlibs};${_fs_c}")
  endif()
endif()

set(_wthttplibs "")
if(NOT "${_thr_c}" STREQUAL "")
  set(_wthttplibs "${_thr_c}")
endif()
if(NOT "${_po_c}" STREQUAL "")
  if(_wthttplibs STREQUAL "")
    set(_wthttplibs "${_po_c}")
  else()
    set(_wthttplibs "${_wthttplibs};${_po_c}")
  endif()
endif()
if(NOT "${_sys_c}" STREQUAL "")
  if(_wthttplibs STREQUAL "")
    set(_wthttplibs "${_sys_c}")
  else()
    set(_wthttplibs "${_wthttplibs};${_sys_c}")
  endif()
endif()
if(NOT "${_fs_c}" STREQUAL "")
  if(_wthttplibs STREQUAL "")
    set(_wthttplibs "${_fs_c}")
  else()
    set(_wthttplibs "${_wthttplibs};${_fs_c}")
  endif()
endif()

file(APPEND "${_config_file}" "set(BOOST_WT_LIBRARIES \"${_wtlibs}\")\n")
file(APPEND "${_config_file}" "set(BOOST_WTHTTP_LIBRARIES \"${_wthttplibs}\")\n")
file(APPEND "${_config_file}" "set(BOOST_SUPPORT_LIBRARIES \"\")\n")

# include the generated targets file from the config so find_package consumers get targets
file(APPEND "${_config_file}" "include(\"\${CMAKE_CURRENT_LIST_DIR}/BoostTargets.cmake\")\n")

# write version file
file(WRITE "${BOOST_CONFIG_DIR}/BoostConfigVersion.cmake" "set(PACKAGE_VERSION \"1.0.0\")\n")

# make discoverable within same configure
list(APPEND CMAKE_PREFIX_PATH "${BOOST_CONFIG_DIR}")
message(STATUS "BoostEmitTargetsAndVars: wrote ${_targets_file} and ${_config_file}")
message(STATUS "BoostEmitTargetsAndVars: appended ${BOOST_CONFIG_DIR} to CMAKE_PREFIX_PATH")
message(STATUS "BoostEmitTargetsAndVars: finished")]]
