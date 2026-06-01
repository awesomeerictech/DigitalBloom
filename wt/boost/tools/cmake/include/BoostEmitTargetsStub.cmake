# BoostEmitTargetsStub.cmake
# Creates INTERFACE IMPORTED Boost::<lib> targets (stubs) so Wt can configure before binaries exist.
# Robust behavior:
# - Creates Boost::<lib> INTERFACE IMPORTED targets immediately (if missing)
# - Sets INTERFACE_INCLUDE_DIRECTORIES safely (resolves ALIAS -> aliased target; falls back to include_directories())
# - Writes BoostTargets.cmake and BoostConfig.cmake stubs in ${CMAKE_BINARY_DIR}/boost
# - Appends ${CMAKE_BINARY_DIR}/boost to CMAKE_PREFIX_PATH so find_package(Boost CONFIG) sees the stubs
cmake_minimum_required(VERSION 3.14)

# ----------------- Configuration -----------------
# Full list of Boost libs (trim or extend as needed)
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

# Where source headers live per-lib (your layout)
set(_SRC_INC_ROOT "${CMAKE_SOURCE_DIR}/boost/libs")

# Where to write generated minimal config so find_package(Boost CONFIG) works
set(BOOST_CONFIG_DIR "${CMAKE_BINARY_DIR}/boost")
file(MAKE_DIRECTORY "${BOOST_CONFIG_DIR}")
set(_targets_file "${BOOST_CONFIG_DIR}/BoostTargets.cmake")
set(_config_file  "${BOOST_CONFIG_DIR}/BoostConfig.cmake")
set(_configver_file "${BOOST_CONFIG_DIR}/BoostConfigVersion.cmake")

# ----------------- Helpers -----------------

# convert to cmake path
function(_to_cmake_path in out)
  if(NOT "${in}" STREQUAL "")
    file(TO_CMAKE_PATH "${in}" _tmp)
    set(${out} "${_tmp}" PARENT_SCOPE)
  else()
    set(${out} "" PARENT_SCOPE)
  endif()
endfunction()

# sanitize lib name to safe token
function(_sanitize_name raw out)
  if("${raw}" STREQUAL "")
    set(${out} "" PARENT_SCOPE)
    return()
  endif()
  string(REGEX REPLACE ".*/" "" _b "${raw}")
  string(REGEX REPLACE "[-\\.].*$" "" _b "${_b}")
  string(REPLACE "-" "_" _b "${_b}")
  string(REGEX REPLACE "[/\\\\:]" "_" _b "${_b}")
  set(${out} "${_b}" PARENT_SCOPE)
endfunction()

# Robust _safe_set_include:
# - If target missing -> create INTERFACE IMPORTED and set include property
# - If target exists and is not ALIAS -> set property on it
# - If target exists and is ALIAS and aliased target exists -> set property on aliased target
# - If target exists and is ALIAS and aliased target missing -> fallback to include_directories() and warn
function(_safe_set_include tgt include_dir)
  if("${tgt}" STREQUAL "")
    return()
  endif()

  if("${include_dir}" STREQUAL "")
    return()
  endif()

  # Convert include_dir to cmake path
  file(TO_CMAKE_PATH "${include_dir}" _inc_p)
  if("${_inc_p}" STREQUAL "")
    return()
  endif()

  # If target missing -> create and set property
  if(NOT TARGET "${tgt}")
    message(STATUS "BoostEmitTargetsStub: target ${tgt} does not exist; creating INTERFACE IMPORTED and setting include")
    add_library("${tgt}" INTERFACE IMPORTED GLOBAL)
    set_target_properties("${tgt}" PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${_inc_p}")
    return()
  endif()

  # target exists; check type
  get_target_property(_t_type "${tgt}" TYPE)
  if(NOT "${_t_type}" STREQUAL "ALIAS")
    # Normal case
    set_target_properties("${tgt}" PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${_inc_p}")
    return()
  endif()

  # ALIAS case: try to resolve aliased real target
  get_target_property(_aliased "${tgt}" ALIASED_TARGET)
  if(NOT "${_aliased}" STREQUAL "" AND TARGET "${_aliased}")
    set_target_properties("${_aliased}" PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${_inc_p}")
    return()
  endif()

  # Fallback: cannot set property on alias -> expose include at directory scope
  message(WARNING "BoostEmitTargetsStub: cannot set include on alias ${tgt} (ALIASED_TARGET is empty or missing). Falling back to include_directories(${_inc_p}) for this directory.")
  include_directories("${_inc_p}")
endfunction()

# ----------------- Create stubs and write files -----------------

# Start writing targets file header (stubs)
file(WRITE "${_targets_file}" "# Auto-generated BoostTargets.cmake (stubs)\ncmake_minimum_required(VERSION 3.14)\n\n")

# Collect include dirs for Boost_INCLUDE_DIRS
set(_collected_inc_dirs "")

foreach(_lib IN LISTS _BOOST_LIB_LIST)
  if("${_lib}" STREQUAL "")
    continue()
  endif()

  _sanitize_name("${_lib}" _san)
  if("${_san}" STREQUAL "")
    message(STATUS "BoostEmitTargetsStub: skipping invalid lib name '${_lib}'")
    continue()
  endif()

  set(_tgt "Boost::${_san}")

  # create/import target if missing
  if(NOT TARGET "${_tgt}")
    add_library("${_tgt}" INTERFACE IMPORTED GLOBAL)
  endif()

  # find per-lib include dir
  set(_inc_dir "${_SRC_INC_ROOT}/${_lib}/include")
  if(EXISTS "${_inc_dir}")
    list(APPEND _collected_inc_dirs "${_inc_dir}")
    # set target include safely
    _safe_set_include("${_tgt}" "${_inc_dir}")
    # write guarded creation to targets file
    _to_cmake_path("${_inc_dir}" _inc_cm)
    file(APPEND "${_targets_file}" "if(NOT TARGET ${_tgt})\n")
    file(APPEND "${_targets_file}" "  add_library(${_tgt} INTERFACE IMPORTED GLOBAL)\n")
    file(APPEND "${_targets_file}" "  set_target_properties(${_tgt} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES \"${_inc_cm}\")\n")
    file(APPEND "${_targets_file}" "endif()\n\n")
  else()
    # no per-lib include; still emit guarded target entry without include
    file(APPEND "${_targets_file}" "if(NOT TARGET ${_tgt})\n")
    file(APPEND "${_targets_file}" "  add_library(${_tgt} INTERFACE IMPORTED GLOBAL)\n")
    file(APPEND "${_targets_file}" "endif()\n\n")
  endif()
endforeach()

# Deduplicate include dirs
list(REMOVE_DUPLICATES _collected_inc_dirs)

# Write BoostConfig.cmake (stubs)
file(WRITE "${_config_file}" "# Auto-generated BoostConfig.cmake (stubs)\ncmake_minimum_required(VERSION 3.14)\nset(Boost_FOUND TRUE)\n")

# Compose Boost_INCLUDE_DIRS
set(_inc_cm "")
foreach(_d IN LISTS _collected_inc_dirs)
  _to_cmake_path("${_d}" _dcm)
  if("${_dcm}" STREQUAL "")
    continue()
  endif()
  if("${_inc_cm}" STREQUAL "")
    set(_inc_cm "${_dcm}")
  else()
    set(_inc_cm "${_inc_cm};${_dcm}")
  endif()
endforeach()

file(APPEND "${_config_file}" "set(Boost_INCLUDE_DIRS \"${_inc_cm}\")\n")
file(APPEND "${_config_file}" "set(Boost_LIBRARY_DIRS \"\")\n") # empty - artifacts not attached by stub

# Wt legacy variables (empty libs)
file(APPEND "${_config_file}" "set(BOOST_INCLUDE_DIRS \"${_inc_cm}\")\n")
file(APPEND "${_config_file}" "set(BOOST_LIB_DIRS \"\")\n")
file(APPEND "${_config_file}" "set(BOOST_WT_LIBRARIES \"\")\n")
file(APPEND "${_config_file}" "set(BOOST_WTHTTP_LIBRARIES \"\")\n")

# include the targets file so find_package(Boost CONFIG) includes stubs
file(APPEND "${_config_file}" "include(\"\${CMAKE_CURRENT_LIST_DIR}/BoostTargets.cmake\")\n")

# Version file
file(WRITE "${_configver_file}" "set(PACKAGE_VERSION \"0.0.0-stub\")\n")

# Make discoverable during same configure
list(APPEND CMAKE_PREFIX_PATH "${BOOST_CONFIG_DIR}")

message(STATUS "BoostEmitTargetsStub: wrote stubs to ${BOOST_CONFIG_DIR} and appended to CMAKE_PREFIX_PATH")
