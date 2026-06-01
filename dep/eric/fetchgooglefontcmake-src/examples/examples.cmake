cmake_minimum_required(VERSION 3.12.0 FATAL_ERROR)

include(../cmake/FetchGoogleFont.cmake)

message(STATUS "Fetch Roboto")
fetch_google_font(
  FAMILY "Roboto"
  STYLES "Black" "BlackItalic" "Bold" "BoldItalic" "Italic" "Light" "LightItalic" "Medium" "MediumItalic" "Regular" "Thin" "ThinItalic"
  VERBOSE
)

message(STATUS "Fetch RobotoCondensed")
fetch_google_font(
  FAMILY "Roboto"
  SUB_FAMILY "Condensed"
  STYLES "Bold" "BoldItalic" "Italic" "Light" "LightItalic" "Medium" "MediumItalic" "Regular"
  VERBOSE
)

message(STATUS "Fetch RobotoMono")
fetch_google_font(
  FAMILY "RobotoMono"
  STYLES "Bold" "BoldItalic" "Italic" "Light" "LightItalic" "Medium" "MediumItalic" "Regular" "Thin" "ThinItalic"
  VERBOSE
)

message(STATUS "Fetch Lato")
fetch_google_font(
  FAMILY "Lato"
  STYLES "Black" "BlackItalic" "Bold" "BoldItalic" "ExtraBold" "ExtraBoldItalic" "ExtraLight" "ExtraLightItalic" "Italic" "Light" "LightItalic" "Medium" "MediumItalic" "Regular" "SemiBold" "SemiBoldItalic" "Thin" "ThinItalic"
  LICENSE "ofl"
  VERBOSE
)

message(STATUS "Fetch OpenSans")
fetch_google_font(
  FAMILY "OpenSans"
  STYLES "Bold" "BoldItalic" "ExtraBold" "ExtraBoldItalic" "Italic" "Light" "LightItalic" "Regular" "SemiBold" "SemiBoldItalic"
  VERBOSE
)