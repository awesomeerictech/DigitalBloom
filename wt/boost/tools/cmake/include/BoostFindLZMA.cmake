# BoostFindLZMA.txt - in-tree xz/liblzma helper with debug prints
message(STATUS "[BoostFindLZMA] Starting search...")

if(DEFINED LIBLZMA_FOUND)
  message(STATUS "[BoostFindLZMA] Already defined LIBLZMA_FOUND=${LIBLZMA_FOUND}, skipping re-search")
  return()
endif()

set(LIBLZMA_FOUND FALSE)
set(LIBLZMA_INCLUDE_DIR "")
set(LIBLZMA_LIBRARIES "")

# ---- Step 1: Prefer in-tree targets ----
foreach(_cand_tgt LibLZMA::LibLZMA liblzma lzma)
  if(TARGET ${_cand_tgt})
    message(STATUS "[BoostFindLZMA] Found target ${_cand_tgt}")
    if(_cand_tgt STREQUAL "liblzma" OR _cand_tgt STREQUAL "lzma")
      if(NOT TARGET LibLZMA::LibLZMA)
        add_library(LibLZMA::LibLZMA ALIAS ${_cand_tgt})
        message(STATUS "[BoostFindLZMA] Created alias LibLZMA::LibLZMA -> ${_cand_tgt}")
      endif()
    endif()

    set(LIBLZMA_FOUND TRUE)
    set(LIBLZMA_LIBRARIES LibLZMA::LibLZMA)

    get_target_property(_incs ${_cand_tgt} INTERFACE_INCLUDE_DIRECTORIES)
    if(_incs)
      message(STATUS "[BoostFindLZMA] INTERFACE_INCLUDE_DIRECTORIES for ${_cand_tgt}: ${_incs}")
      foreach(_i IN LISTS _incs)
        if(NOT "${_i}" MATCHES "\\$<" AND IS_DIRECTORY "${_i}")
          set(LIBLZMA_INCLUDE_DIR "${_i}")
          message(STATUS "[BoostFindLZMA] Using include dir from target: ${LIBLZMA_INCLUDE_DIR}")
          break()
        endif()
      endforeach()
    endif()
    break()
  endif()
endforeach()

# ---- Step 2: fallback to known source include dirs ----
if(NOT LIBLZMA_FOUND OR NOT LIBLZMA_INCLUDE_DIR)
  message(STATUS "[BoostFindLZMA] Falling back to source include directories...")
  set(_cands
    "${CMAKE_SOURCE_DIR}/wt/boost/xz/lib"
    "${CMAKE_SOURCE_DIR}/wt/boost/xz/lib/src"
    "${CMAKE_SOURCE_DIR}/wt/boost/xz/include"
  )
  foreach(_cand IN LISTS _cands)
    if(EXISTS "${_cand}" AND IS_DIRECTORY "${_cand}")
      file(GLOB_RECURSE _hdrs "${_cand}/*.[h|hpp|ipp|inl]")
      if(_hdrs)
        message(STATUS "[BoostFindLZMA] Found headers under ${_cand}")
        set(LIBLZMA_INCLUDE_DIR "${_cand}")
        set(LIBLZMA_FOUND TRUE)
        break()
      endif()
    endif()
  endforeach()
endif()

# ---- Step 3: export variables ----
if(LIBLZMA_FOUND)
  message(STATUS "[BoostFindLZMA] SUCCESS: LIBLZMA_FOUND=TRUE")
  message(STATUS "[BoostFindLZMA] LIBLZMA_INCLUDE_DIR=${LIBLZMA_INCLUDE_DIR}")
  message(STATUS "[BoostFindLZMA] LIBLZMA_LIBRARIES=${LIBLZMA_LIBRARIES}")

  set(LIBLZMA_INCLUDE_DIRS "${LIBLZMA_INCLUDE_DIR}" CACHE PATH "liblzma include (auto-detected)" FORCE)
  set(LIBLZMA_LIBRARIES "${LIBLZMA_LIBRARIES}" CACHE INTERNAL "liblzma libraries (target or empty)")
  set(LIBLZMA_FOUND TRUE CACHE BOOL "liblzma found (shim)" FORCE)
else()
  message(WARNING "[BoostFindLZMA] FAILED: Could not locate liblzma target or includes!")
endif()
