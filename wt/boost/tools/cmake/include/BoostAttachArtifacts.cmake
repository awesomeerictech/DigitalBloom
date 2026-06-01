# BoostAttachArtifacts.cmake
cmake_minimum_required(VERSION 3.14)

# Where boost artifacts live relative to build dir
set(_artifact_root "${CMAKE_BINARY_DIR}/boost/libs")

# Where generated config lives
set(BOOST_CONFIG_DIR "${CMAKE_BINARY_DIR}/boost")
set(_targets_file "${BOOST_CONFIG_DIR}/BoostTargets.cmake")
set(_config_file  "${BOOST_CONFIG_DIR}/BoostConfig.cmake")

# components you support (should match stub list)
set(_BOOST_LIB_LIST
  filesystem thread system program_options # (same full list)
  # ...
)

# collectors for config variables
set(_lib_dirs "")
set(_component_map "")

foreach(_lib IN LISTS _BOOST_LIB_LIST)
  string(REPLACE "-" "_" _san "${_lib}")
  set(_perdir "${_artifact_root}/${_lib}")
  set(_artifact "")

  if(EXISTS "${_perdir}")
    file(GLOB _c "${_perdir}/libboost_${_lib}*" "${_perdir}/boost_${_lib}*" "${_perdir}/${_lib}*")
    if(_c)
      list(SORT _c)
      list(GET _c 0 _f)
      set(_artifact "${_perdir}/${_f}")
    endif()
  endif()

  # If not in perdir, try root fallback
  if(NOT _artifact)
    file(GLOB _c2 "${_artifact_root}/libboost_${_lib}*" "${_artifact_root}/boost_${_lib}*" "${_artifact_root}/${_lib}*")
    if(_c2)
      list(SORT _c2)
      list(GET _c2 0 _f2)
      set(_artifact "${_artifact_root}/${_f2}")
    endif()
  endif()

  if(_artifact)
    # convert path
    file(TO_CMAKE_PATH "${_artifact}" _art_c)
    # ensure target exists
    if(NOT TARGET Boost::${_san})
      add_library(Boost::${_san} INTERFACE IMPORTED GLOBAL)
    endif()
    # set imported location
    set_target_properties(Boost::${_san} PROPERTIES IMPORTED_LOCATION "${_art_c}")
    set_property(TARGET Boost::${_san} APPEND PROPERTY INTERFACE_LINK_LIBRARIES "${_art_c}")

    # record for config vars
    get_filename_component(_ad "${_artifact}" DIRECTORY)
    list(APPEND _lib_dirs "${_ad}")
    set(_component_map "${_component_map};${_lib}=${_artifact}")
    message(STATUS "BoostAttachArtifacts: attached ${_artifact} to Boost::${_san}")
  endif()
endforeach()

# Deduplicate lib dirs and update Boost_LIBRARY_DIRS
list(REMOVE_DUPLICATES _lib_dirs)
set(_lib_list_cm "")
foreach(_d IN LISTS _lib_dirs)
  file(TO_CMAKE_PATH "${_d}" _dcm)
  if(_lib_list_cm STREQUAL "") set(_lib_list_cm "${_dcm}") else set(_lib_list_cm "${_lib_list_cm};${_dcm}") endif()
endforeach()

# Update BoostConfig.cmake variables by rewriting the config (keeps targets include)
file(WRITE "${_config_file}" "# Auto-generated BoostConfig.cmake (updated with artifacts)\ncmake_minimum_required(VERSION 3.14)\nset(Boost_FOUND TRUE)\n")
file(APPEND "${_config_file}" "set(Boost_INCLUDE_DIRS \"${Boost_INCLUDE_DIRS}\")\n")
file(APPEND "${_config_file}" "set(Boost_LIBRARY_DIRS \"${_lib_list_cm}\")\n")

# write some per-component variables if present (filesystem, system, thread, program_options)
function(_find_comp comp out)
  set(_res "")
  if(_component_map)
    foreach(_it IN LISTS _component_map)
      string(FIND "${_it}" "=" _eq)
      if(_eq GREATER -1)
        string(SUBSTRING "${_it}" 0 ${_eq} _k)
        math(EXPR _vstart "${_eq} + 1")
        string(SUBSTRING "${_it}" ${_vstart} -1 _v)
        if(_k STREQUAL "${comp}")
          set(_res "${_v}")
          break()
        endif()
      endif()
    endforeach()
  endif()
  set(${out} "${_res}" PARENT_SCOPE)
endfunction()

_find_comp("filesystem" _fs)
_find_comp("program_options" _po)
_find_comp("system" _sys)
_find_comp("thread" _thr)

file(APPEND "${_config_file}" "set(Boost_FILESYSTEM_LIBRARY \"${_fs}\")\n")
file(APPEND "${_config_file}" "set(Boost_PROGRAM_OPTIONS_LIBRARY \"${_po}\")\n")
file(APPEND "${_config_file}" "set(Boost_SYSTEM_LIBRARY \"${_sys}\")\n")
file(APPEND "${_config_file}" "set(Boost_THREAD_LIBRARY \"${_thr}\")\n")

# preserve Wt legacy variable names for compatibility
file(APPEND "${_config_file}" "set(BOOST_LIB_DIRS \"${_lib_list_cm}\")\n")
# Compose BOOST_WT_LIBRARIES from the component vars (thread;system;filesystem)
set(_wtlibs "")
if(NOT "${_thr}" STREQUAL "") set(_wtlibs "${_thr}") endif()
if(NOT "${_sys}" STREQUAL "") if(_wtlibs STREQUAL "") set(_wtlibs "${_sys}") else set(_wtlibs "${_wtlibs};${_sys}") endif() endif()
if(NOT "${_fs}" STREQUAL "") if(_wtlibs STREQUAL "") set(_wtlibs "${_fs}") else set(_wtlibs "${_wtlibs};${_fs}") endif() endif()
file(APPEND "${_config_file}" "set(BOOST_WT_LIBRARIES \"${_wtlibs}\")\n")

# ensure BoostTargets.cmake is included at end
file(APPEND "${_config_file}" "include(\"\${CMAKE_CURRENT_LIST_DIR}/BoostTargets.cmake\")\n")

# update version file
file(WRITE "${BOOST_CONFIG_DIR}/BoostConfigVersion.cmake" "set(PACKAGE_VERSION \"1.0.0\")\n")

message(STATUS "BoostAttachArtifacts: updated ${_config_file} with attached artifacts (if any)")
