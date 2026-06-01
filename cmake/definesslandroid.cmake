# --- OpenSSL for Android helper ------------------------------------------------
# Place this before any targets needing OpenSSL or before find_package(OpenSSL)
# Folder structure expected:
#   ${CMAKE_SOURCE_DIR}/opensslandroid/<abi>/lib/libcrypto.(so|a)
#   ${CMAKE_SOURCE_DIR}/opensslandroid/<abi>/lib/libssl.(so|a)
#   optional: ${CMAKE_SOURCE_DIR}/opensslandroid/<abi>/include
#
# Supported ABIs: armeabi-v7a, arm64-v8a, x86, x86_64

if(DEFINED CMAKE_ANDROID_ARCH_ABI)
  set(_android_abi "${CMAKE_ANDROID_ARCH_ABI}")
elseif(DEFINED ANDROID_ABI)
  set(_android_abi "${ANDROID_ABI}")
else()
  set(_android_abi "")
endif()

# Map toolchain ABI names to directory names
set(_abi_map_armeabi-v7a "armeabi-v7a")
set(_abi_map_arm64-v8a   "arm64-v8a")
set(_abi_map_x86         "x86")
set(_abi_map_x86_64      "x86_64")

# If ABI not set, allow manual override
if(_android_abi STREQUAL "" AND DEFINED FORCED_OPENSSL_ANDROID_ABI)
  set(_android_abi "${FORCED_OPENSSL_ANDROID_ABI}")
endif()

# Determine ABI folder name
if(_android_abi STREQUAL "armeabi-v7a")
  set(_abi_folder "${_abi_map_armeabi-v7a}")
elseif(_android_abi STREQUAL "arm64-v8a" OR _android_abi STREQUAL "aarch64")
  set(_abi_folder "${_abi_map_arm64-v8a}")
elseif(_android_abi STREQUAL "x86")
  set(_abi_folder "${_abi_map_x86}")
elseif(_android_abi STREQUAL "x86_64")
  set(_abi_folder "${_abi_map_x86_64}")
else()
  set(_abi_folder "${_android_abi}")
endif()

set(_openssl_base "${CMAKE_SOURCE_DIR}/opensslandroid")

# Candidate ABI folders (fallback search order if unknown ABI)
if(_abi_folder)
  set(_candidate_folders "${_openssl_base}/${_abi_folder}")
else()
  set(_candidate_folders
    "${_openssl_base}/armeabi-v7a"
    "${_openssl_base}/arm64-v8a"
    "${_openssl_base}/x86"
    "${_openssl_base}/x86_64"
  )
endif()

set(_found_crypto "")
set(_found_ssl "")
set(_found_include "")

foreach(_dir IN LISTS _candidate_folders)
  if(EXISTS "${_dir}/lib")
    if(NOT _found_crypto)
      if(EXISTS "${_dir}/lib/libcrypto_3.so")
        set(_found_crypto "${_dir}/lib/libcrypto_3.so")
      elseif(EXISTS "${_dir}/lib/libcrypto.a")
        set(_found_crypto "${_dir}/lib/libcrypto.a")
      endif()
    endif()
    if(NOT _found_ssl)
      if(EXISTS "${_dir}/lib/libssl_3.so")
        set(_found_ssl "${_dir}/lib/libssl_3.so")
      elseif(EXISTS "${_dir}/lib/libssl.a")
        set(_found_ssl "${_dir}/lib/libssl.a")
      endif()
    endif()
    if(NOT _found_include AND EXISTS "${_dir}/include")
      set(_found_include "${_dir}/include")
    endif()
  endif()
  if(_found_crypto AND _found_ssl)
    break()
  endif()
endforeach()

# Cache variables for find_package(OpenSSL)
if(_found_crypto)
  set(OPENSSL_CRYPTO_LIBRARY "${_found_crypto}" CACHE STRING "OpenSSL crypto library (Android)" FORCE)
endif()
if(_found_ssl)
  set(OPENSSL_SSL_LIBRARY "${_found_ssl}" CACHE STRING "OpenSSL ssl library (Android)" FORCE)
endif()
if(_found_include)
  set(OPENSSL_INCLUDE_DIR "${_found_include}" CACHE STRING "OpenSSL include directory (Android)" FORCE)
endif()
set(OPENSSL_ROOT_DIR "${_openssl_base}" CACHE STRING "OpenSSL root directory (Android)" FORCE)

# --- Result reporting / fallback messaging ------------------------------------
if(NOT _found_crypto OR NOT _found_ssl OR NOT _found_include)
  message(WARNING "")
  message(WARNING "Could NOT find OpenSSL, try to set the path to OpenSSL root folder in the system variable OPENSSL_ROOT_DIR "
                  "(missing: "
                  "$<IF:$<BOOL:${_found_crypto}>,,OPENSSL_CRYPTO_LIBRARY >"
                  "$<IF:$<BOOL:${_found_ssl}>,,OPENSSL_SSL_LIBRARY >"
                  "$<IF:$<BOOL:${_found_include}>,,OPENSSL_INCLUDE_DIR>"
                  ")")
  message(WARNING "")
else()
  message(STATUS "Using OpenSSL for Android:")
  message(STATUS "  ABI: ${_abi_folder}")
  message(STATUS "  OPENSSL_CRYPTO_LIBRARY = ${OPENSSL_CRYPTO_LIBRARY}")
  message(STATUS "  OPENSSL_SSL_LIBRARY    = ${OPENSSL_SSL_LIBRARY}")
  message(STATUS "  OPENSSL_INCLUDE_DIR    = ${OPENSSL_INCLUDE_DIR}")
endif()
# -------------------------------------------------------------------------------
