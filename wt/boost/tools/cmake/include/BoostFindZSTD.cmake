# BoostFindZSTD.txt - in-tree zstd helper with debug prints
message(STATUS "[BoostFindZSTD] Starting search...")

if(DEFINED ZSTD_FOUND)
  message(STATUS "[BoostFindZSTD] Already defined ZSTD_FOUND=${ZSTD_FOUND}, skipping re-search")
  return()
endif()

set(ZSTD_FOUND FALSE)
set(ZSTD_INCLUDE_DIRS "")
set(ZSTD_LIBRARIES "")

# ---- Step 1: Prefer in-tree targets ----
foreach(_cand_tgt zstd::libzstd libzstd)
  if(TARGET ${_cand_tgt})
    message(STATUS "[BoostFindZSTD] Found target ${_cand_tgt}")
    if(_cand_tgt STREQUAL "libzstd" AND NOT TARGET zstd::libzstd)
      add_library(zstd::libzstd ALIAS libzstd)
      message(STATUS "[BoostFindZSTD] Created alias zstd::libzstd -> libzstd")
    endif()

    set(ZSTD_FOUND TRUE)
    set(ZSTD_LIBRARIES zstd::libzstd)

    get_target_property(_incs ${_cand_tgt} INTERFACE_INCLUDE_DIRECTORIES)
    if(_incs)
      message(STATUS "[BoostFindZSTD] INTERFACE_INCLUDE_DIRECTORIES for ${_cand_tgt}: ${_incs}")
      foreach(_i IN LISTS _incs)
        if(NOT "${_i}" MATCHES "\\$<" AND IS_DIRECTORY "${_i}")
          set(ZSTD_INCLUDE_DIRS "${_i}")
          message(STATUS "[BoostFindZSTD] Using include dir from target: ${ZSTD_INCLUDE_DIRS}")
          break()
        endif()
      endforeach()
    endif()
    break()
  endif()
endforeach()

# ---- Step 2: fallback to known source include dirs ----
if(NOT ZSTD_FOUND OR NOT ZSTD_INCLUDE_DIRS)
  message(STATUS "[BoostFindZSTD] Falling back to source include directories...")
  set(_cands
    "${CMAKE_SOURCE_DIR}/wt/boost/zstd/lib"
    "${CMAKE_SOURCE_DIR}/wt/boost/zstd/zlibWrapper"
    "${CMAKE_SOURCE_DIR}/wt/boost/zstd/include"
  )
  foreach(_cand IN LISTS _cands)
    if(EXISTS "${_cand}" AND IS_DIRECTORY "${_cand}")
      file(GLOB_RECURSE _hdrs "${_cand}/*.[h|hpp|ipp|inl]")
      if(_hdrs)
        message(STATUS "[BoostFindZSTD] Found headers under ${_cand}")
        set(ZSTD_INCLUDE_DIRS "${_cand}")
        set(ZSTD_FOUND TRUE)
        break()
      endif()
    endif()
  endforeach()
endif()

# ---- Step 3: export variables ----
if(ZSTD_FOUND)
  message(STATUS "[BoostFindZSTD] SUCCESS: ZSTD_FOUND=TRUE")
  message(STATUS "[BoostFindZSTD] ZSTD_INCLUDE_DIRS=${ZSTD_INCLUDE_DIRS}")
  message(STATUS "[BoostFindZSTD] ZSTD_LIBRARIES=${ZSTD_LIBRARIES}")

  set(ZSTD_INCLUDE_DIR "${ZSTD_INCLUDE_DIRS}" CACHE PATH "zstd include (auto-detected)" FORCE)
  set(ZSTD_LIBRARIES "${ZSTD_LIBRARIES}" CACHE INTERNAL "zstd libraries (target or empty)")
  set(ZSTD_FOUND TRUE CACHE BOOL "zstd found (shim)" FORCE)
else()
  message(WARNING "[BoostFindZSTD] FAILED: Could not locate zstd target or includes!")
endif()
