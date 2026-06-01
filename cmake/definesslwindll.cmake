# --- OpenSSL for Windows helper (DLL-preferred) --------------------------------
if(WIN32)
  # base path where you keep Windows prebuilt OpenSSL
  set(_openssl_win_base "${CMAKE_SOURCE_DIR}/opensslwin")

  # determine pointer size to guess x86 vs x64
  if(DEFINED CMAKE_SIZEOF_VOID_P)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
      set(_arch_tag "x64")
    else()
      set(_arch_tag "x86")
    endif()
  else()
    set(_arch_tag "x64")
  endif()

  # candidate dirs
  set(_lib_dir "${_openssl_win_base}/lib")
  set(_bin_dir "${_openssl_win_base}/bin")
  set(_include_dir "${_openssl_win_base}/include")

  set(_found_crypto_lib "")
  set(_found_ssl_lib "")
  set(_found_crypto_dll "")
  set(_found_ssl_dll "")
  set(_found_include "")

  # 1) Prefer DLLs in bin/ (force DLL over .lib if both exist)
  if(EXISTS "${_bin_dir}")
    file(GLOB _crypto_dll_candidates "${_bin_dir}/libcrypto*-${_arch_tag}.dll" "${_bin_dir}/libcrypto*.dll")
    list(SORT _crypto_dll_candidates)
    list(GET _crypto_dll_candidates 0 _first_crypto_dll QUIET)
    if(_first_crypto_dll)
      set(_found_crypto_dll "${_first_crypto_dll}")
    endif()

    file(GLOB _ssl_dll_candidates "${_bin_dir}/libssl*-${_arch_tag}.dll" "${_bin_dir}/libssl*.dll")
    list(SORT _ssl_dll_candidates)
    list(GET _ssl_dll_candidates 0 _first_ssl_dll QUIET)
    if(_first_ssl_dll)
      set(_found_ssl_dll "${_first_ssl_dll}")
    endif()
  endif()

  # 2) Still record .lib import libs if present (but do NOT prefer them)
  if(EXISTS "${_lib_dir}")
    if(EXISTS "${_lib_dir}/libcrypto.lib")
      set(_found_crypto_lib "${_lib_dir}/libcrypto.lib")
    else()
      file(GLOB _crypto_lib_candidates "${_lib_dir}/libcrypto*.lib")
      list(SORT _crypto_lib_candidates)
      list(REVERSE _crypto_lib_candidates)
      list(GET _crypto_lib_candidates 0 _first_crypto_lib QUIET)
      if(_first_crypto_lib)
        set(_found_crypto_lib "${_first_crypto_lib}")
      endif()
    endif()

    if(EXISTS "${_lib_dir}/libssl.lib")
      set(_found_ssl_lib "${_lib_dir}/libssl.lib")
    else()
      file(GLOB _ssl_lib_candidates "${_lib_dir}/libssl*.lib")
      list(SORT _ssl_lib_candidates)
      list(REVERSE _ssl_lib_candidates)
      list(GET _ssl_lib_candidates 0 _first_ssl_lib QUIET)
      if(_first_ssl_lib)
        set(_found_ssl_lib "${_first_ssl_lib}")
      endif()
    endif()
  endif()

  # 3) Include dir
  if(EXISTS "${_include_dir}")
    set(_found_include "${_include_dir}")
  endif()

  # 4) Populate CACHE variables - prefer DLLs over .lib (user asked to force DLL use)
  if(_found_crypto_dll)
    set(OPENSSL_CRYPTO_LIBRARY "${_found_crypto_dll}" CACHE STRING "OpenSSL crypto dll (Windows) - preferred" FORCE)
  elseif(_found_crypto_lib)
    set(OPENSSL_CRYPTO_LIBRARY "${_found_crypto_lib}" CACHE STRING "OpenSSL crypto import lib (Windows) - fallback" FORCE)
  endif()

  if(_found_ssl_dll)
    set(OPENSSL_SSL_LIBRARY "${_found_ssl_dll}" CACHE STRING "OpenSSL ssl dll (Windows) - preferred" FORCE)
  elseif(_found_ssl_lib)
    set(OPENSSL_SSL_LIBRARY "${_found_ssl_lib}" CACHE STRING "OpenSSL ssl import lib (Windows) - fallback" FORCE)
  endif()

  if(_found_include)
    set(OPENSSL_INCLUDE_DIR "${_found_include}" CACHE STRING "OpenSSL include dir (Windows)" FORCE)
  endif()

  if(EXISTS "${_openssl_win_base}")
    set(OPENSSL_ROOT_DIR "${_openssl_win_base}" CACHE STRING "OpenSSL root dir (Windows)" FORCE)
  endif()

  # Build missing list for message (space-separated)
  set(_missing_list "")
  if(NOT DEFINED OPENSSL_CRYPTO_LIBRARY OR OPENSSL_CRYPTO_LIBRARY STREQUAL "")
    list(APPEND _missing_list "OPENSSL_CRYPTO_LIBRARY")
  endif()
  if(NOT DEFINED OPENSSL_INCLUDE_DIR OR OPENSSL_INCLUDE_DIR STREQUAL "")
    list(APPEND _missing_list "OPENSSL_INCLUDE_DIR")
  endif()
  if(NOT DEFINED OPENSSL_SSL_LIBRARY OR OPENSSL_SSL_LIBRARY STREQUAL "")
    list(APPEND _missing_list "OPENSSL_SSL_LIBRARY")
  endif()
  string(REPLACE ";" " " _missing_list_str "${_missing_list}")

  # Messages & warning about DLL-first policy
  message(STATUS "OpenSSL (Windows) search — DLL preferred:")
  message(STATUS "  Architecture guessed: ${_arch_tag}")
  message(STATUS "  Base dir: ${_openssl_win_base}")
  message(STATUS "  lib dir: ${_lib_dir}")
  message(STATUS "  bin dir: ${_bin_dir}")
  message(STATUS "  include: ${_found_include}")

  message(STATUS "  OPENSSL_CRYPTO_LIBRARY = ${OPENSSL_CRYPTO_LIBRARY}")
  message(STATUS "  OPENSSL_SSL_LIBRARY    = ${OPENSSL_SSL_LIBRARY}")
  message(STATUS "  OPENSSL_INCLUDE_DIR    = ${OPENSSL_INCLUDE_DIR}")
  message(STATUS "  OPENSSL_ROOT_DIR       = ${OPENSSL_ROOT_DIR}")

  if(_missing_list_str)
    message(WARNING "Could NOT find OpenSSL, try to set the path to OpenSSL root folder in the system variable OPENSSL_ROOT_DIR (missing: ${_missing_list_str})")
  endif()

  if(_found_crypto_dll OR _found_ssl_dll)
    message(WARNING "Note: this configuration prefers DLL paths for OPENSSL_*_LIBRARY. MSVC usually links against import .lib files;"
                    " if you intend to link at build time, ensure import libs are available or create imported targets. If you only need the DLLs at runtime,"
                    " this will locate them for packaging/copying.")
  endif()

  # Optionally call find_package(OpenSSL) after setting these vars
  # find_package(OpenSSL REQUIRED)
endif()
# -------------------------------------------------------------------------------
