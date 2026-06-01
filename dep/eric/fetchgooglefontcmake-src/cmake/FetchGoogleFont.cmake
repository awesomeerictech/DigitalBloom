#
# MIT License
#
# Copyright (c) 2020 Olivier Le Doeuff
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

include(CMakeParseArguments)

# CMake function to download google fonts at configure time from [google/fonts](https://github.com/google/fonts).
#
# Example:
#
# ```cmake
# # Download Roboto-Regular.ttf & Roboto-Medium.ttf
# fetch_google_font(
#   FAMILY "Roboto"
#   STYLES "Medium" "Regular"
#   OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/Fonts
#   VERBOSE
# )
# ```
#
# Usage: `fetch_google_font([options...])`
#
# * `VERBOSE`: Log a message for each ttf file downloaded.
# * `FAMILY`: Family of the font.
# * `SUB_FAMILY`: Sub Family of the font. *for example, `Condensed` in family `Roboto`*.
# * `STYLES`: List of styles to download, this is a list. For example `Regular`, `Medium`, `BlackItalic`, ...
# * `LICENSE`: The font license. By default `apache`.
# * `TAG`: Repository tag. By default `master`.
# * `URL`: Url repository which to download, by default `https://github.com/google/fonts/`
# * `OUTPUT_DIR`: The directory in which the font will be downloaded. By default `CMAKE_CURRENT_BINARY_DIR`.
#
function(fetch_google_font)

  set(GFONT_OPTIONS VERBOSE)
  set(GFONT_ONE_VALUE_ARG FAMILY SUB_FAMILY OUTPUT_DIR URL TAG LICENSE)
  set(GFONT_MULTI_VALUE_ARG STYLES)

  # parse the function arguments
  cmake_parse_arguments(ARGGFONT "${GFONT_OPTIONS}" "${GFONT_ONE_VALUE_ARG}" "${GFONT_MULTI_VALUE_ARG}" ${ARGN})

  # Make sure tag have a default value
  if(NOT ARGGFONT_TAG OR ARGGFONT_TAG STREQUAL "")
    set(ARGGFONT_TAG "master")
  endif()

  # Make sure tag have a default value
  if(NOT ARGGFONT_LICENSE OR ARGGFONT_LICENSE STREQUAL "")
    set(ARGGFONT_LICENSE "apache")
  endif()

  # Make sure url have a default value
  if(NOT ARGGFONT_URL OR ARGGFONT_URL STREQUAL "")
    set(ARGGFONT_URL "https://github.com/google/fonts")
  endif()
  set(BASE_URL ${ARGGFONT_URL}/raw/${ARGGFONT_TAG}/${ARGGFONT_LICENSE})

  # Default OUTPUT_DIR to the binary dir
  if(NOT ARGGFONT_OUTPUT_DIR OR ARGGFONT_OUTPUT_DIR STREQUAL "")
    set(ARGGFONT_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
  endif()

  # When downloading roboto fonts, they are located in an extra 'static' folder
  string(FIND ${ARGGFONT_FAMILY} "Roboto" ROBOTO_LOC)
  if(NOT ${ROBOTO_LOC} EQUAL -1)
    set(URL_EXTRA_FOLDER "static")
  endif()

  # Download each style
  foreach(STYLE ${ARGGFONT_STYLES})

    # Create url for the font
    string(TOLOWER ${ARGGFONT_FAMILY} FAMILY_TOLOWER)
    set(FONT_FILENAME "${ARGGFONT_FAMILY}${ARGGFONT_SUB_FAMILY}-${STYLE}.ttf")
    set(FONT_URL "${BASE_URL}/${FAMILY_TOLOWER}/${URL_EXTRA_FOLDER}/${FONT_FILENAME}")
    set(FONT_FILEPATH ${ARGGFONT_OUTPUT_DIR}/${FONT_FILENAME})

    # Check that previous download went ok.
    if(EXISTS ${FONT_FILEPATH})
      file(SIZE ${FONT_FILEPATH} FONT_FILESIZE)
    else()
      set(FONT_FILESIZE 0)
    endif()

    # Only download if file doesn't exist or previous donwload not finished
    if(NOT EXISTS ${FONT_FILEPATH} OR ${FONT_FILESIZE} EQUAL 0)

      # Download the font
      if(ARGGFONT_VERBOSE)
        message(STATUS "Download ${FONT_FILENAME} from ${FONT_URL}")
      endif()
      file(DOWNLOAD ${FONT_URL} ${FONT_FILEPATH})

      # Check that download went ok. Remove the empty file if failed
      file(SIZE ${FONT_FILEPATH} FONT_FILESIZE)
      if(${FONT_FILESIZE} EQUAL 0)
        message(WARNING "Fail to download ${FONT_URL}. File doesn't exists.")
        file(REMOVE ${FONT_FILEPATH})
      else()
        if(ARGGFONT_VERBOSE)
          message(STATUS "${FONT_FILENAME} size is ${FONT_FILESIZE} bytes")
        endif()
      endif()

    endif()

  endforeach()

endfunction()
