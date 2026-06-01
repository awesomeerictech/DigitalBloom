# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "C:/Qt/dev/qaterialdev/build/Desktop_Qt_6_9_3_MSVC2022_64bit-Release/_deps/qaterial-src")
  file(MAKE_DIRECTORY "C:/Qt/dev/qaterialdev/build/Desktop_Qt_6_9_3_MSVC2022_64bit-Release/_deps/qaterial-src")
endif()
file(MAKE_DIRECTORY
  "C:/Qt/dev/qaterialdev/build/Desktop_Qt_6_9_3_MSVC2022_64bit-Release/_deps/qaterial-build"
  "C:/Qt/dev/qaterialdev/build/Desktop_Qt_6_9_3_MSVC2022_64bit-Release/_deps/qaterial-subbuild/qaterial-populate-prefix"
  "C:/Qt/dev/qaterialdev/build/Desktop_Qt_6_9_3_MSVC2022_64bit-Release/_deps/qaterial-subbuild/qaterial-populate-prefix/tmp"
  "C:/Qt/dev/qaterialdev/build/Desktop_Qt_6_9_3_MSVC2022_64bit-Release/_deps/qaterial-subbuild/qaterial-populate-prefix/src/qaterial-populate-stamp"
  "C:/Qt/dev/qaterialdev/build/Desktop_Qt_6_9_3_MSVC2022_64bit-Release/_deps/qaterial-subbuild/qaterial-populate-prefix/src"
  "C:/Qt/dev/qaterialdev/build/Desktop_Qt_6_9_3_MSVC2022_64bit-Release/_deps/qaterial-subbuild/qaterial-populate-prefix/src/qaterial-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Qt/dev/qaterialdev/build/Desktop_Qt_6_9_3_MSVC2022_64bit-Release/_deps/qaterial-subbuild/qaterial-populate-prefix/src/qaterial-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Qt/dev/qaterialdev/build/Desktop_Qt_6_9_3_MSVC2022_64bit-Release/_deps/qaterial-subbuild/qaterial-populate-prefix/src/qaterial-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
