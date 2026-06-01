# 🔤 FetchGoogleFontCMake

![gfonts](https://www.fasterize.com/wp-content/uploads/2019/12/googlefont.png)

CMake function to download google fonts at configure time from [google/fonts](https://github.com/google/fonts).

Example:

```cmake
fetch_google_font(
  FAMILY "Roboto"
  STYLES "Medium" "Regular"
  OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/Fonts
  VERBOSE
)
```

Usage: `fetch_google_font([options...])`

* `VERBOSE`: Log a message for each ttf file downloaded.
* `FAMILY`: Family of the font.
* `SUB_FAMILY`: Sub Family of the font. *for example, `Condensed` in family `Roboto`*.
* `STYLES`: List of styles to download, this is a list. For example `Regular`, `Medium`, `BlackItalic`, ...
* `LICENSE`: The font license. By default `apache`.
* `TAG`: Repository tag. By default `master`.
* `URL`: Url repository which to download, by default `https://github.com/google/fonts`
* `OUTPUT_DIR`: The directory in which the font will be downloaded. By default `CMAKE_CURRENT_BINARY_DIR`.

## [🗜️](https://emojipedia.org/clamp/) Examples

Try examples with `cmake -P examples/examples.cmake`.

### Download Roboto

```cmake
fetch_google_font(
  FAMILY "Roboto"
  STYLES "Black" "BlackItalic" "Bold" "BoldItalic" "Italic" "Light" "LightItalic" "Medium" "MediumItalic" "Regular" "Thin" "ThinItalic"
)
```

### Download Roboto Condensed

```cmake
fetch_google_font(
  FAMILY "Roboto"
  SUB_FAMILY "Condensed"
  STYLES "Bold" "BoldItalic" "Italic" "Light" "LightItalic" "Medium" "MediumItalic" "Regular"
)
```

### Download Roboto Mono

```cmake
fetch_google_font(
  FAMILY "RobotoMono"
  STYLES "Bold" "BoldItalic" "Italic" "Light" "LightItalic" "Medium" "MediumItalic" "Regular" "Thin" "ThinItalic"
)
```

### Download Lato

```cmake
fetch_google_font(
  FAMILY "Lato"
  STYLES "Black" "BlackItalic" "Bold" "BoldItalic" "ExtraBold" "ExtraBoldItalic" "ExtraLight" "ExtraLightItalic" "Italic" "Light" "LightItalic" "Medium" "MediumItalic" "Regular" "SemiBold" "SemiBoldItalic" "Thin" "ThinItalic"
  LICENSE "ofl"
)
```

### Download from custom repo

```cmake
fetch_google_font(
  FAMILY "Lato"
  STYLES "Regular"
  LICENSE "ofl"
  URL "https://github.com/OlivierLDff/fonts"
  TAG "master"
)
```

## 🔨 How to integrate in your CMake project

They are multiple way of integrating this function in your project. I would suggest the **FetchContent** method, as you will have updates and bug fixes.

### FetchContent

Use the `FetchContent` command in any of your CMake files to download this repo. The function will immediately be available.

```cmake
include(FetchContent)

set(FETCHGOOGLEFONTCMAKE_REPOSITORY "https://github.com/OlivierLDff/FetchGoogleFontCMake.git" CACHE STRING "FetchGoogleFontCMake repository, can be a local URL")
set(FETCHGOOGLEFONTCMAKE_TAG "master" CACHE STRING "FetchGoogleFontCMake git tag")

FetchContent_Declare(
  FetchGoogleFontCMake
  GIT_REPOSITORY ${FETCHGOOGLEFONTCMAKE_REPOSITORY}
  GIT_TAG        ${FETCHGOOGLEFONTCMAKE_TAG}
  GIT_SHALLOW    1
)

FetchContent_MakeAvailable(FetchGoogleFontCMake)
```

This file is available in `cmake/FetchGoogleFontCMake.cmake`. It can be used like that:

```cmake
include(path/to/FetchGoogleFontCMake.cmake)
fetch_google_font(
  FAMILY "Roboto"
  STYLES "Regular"
)
```

### Copy `cmake/FetchGoogleFont.cmake` to your sources

Then:

```cmake
include(path/to/FetchGoogleFont.cmake)
fetch_google_font(
  FAMILY "Roboto"
  STYLES "Regular"
)
```

