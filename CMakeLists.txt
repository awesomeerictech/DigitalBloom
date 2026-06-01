cmake_minimum_required(VERSION 3.16)

#[[set(CPM_USE_LOCAL_PACKAGES ON CACHE BOOL "Force my bool variable to OFF" FORCE)
#set(CPM_LOCAL_PACKAGES_ONLY ON CACHE BOOL "Force my bool variable to OFF" FORCE)
set(CPM_DOWNLOAD_ALL OFF CACHE BOOL "Force my bool variable to OFF" FORCE)
set(CPM_DONT_UPDATE_MODULE_PATH ON CACHE BOOL "Force my bool variable to OFF" FORCE)
#set(MaterialDesignIcons "${CMAKE_BINARY_DIR}/eric/materialdesignicons-src" CACHE INTERNAL "MaterialDesignIcons" )
set(MATERIALDESIGNICONS_ICONS_DIR "${CMAKE_BINARY_DIR}/eric/materialdesignicons-src/svg" CACHE INTERNAL "MaterialDesignIcons" )
set(QOlm "${CMAKE_BINARY_DIR}/eric/qolm-src/cmake" CACHE INTERNAL "QOlm" )
set(FetchGoogleFontCMake "${CMAKE_BINARY_DIR}/eric/fetchgooglefontcmake-src/cmake" CACHE INTERNAL "FetchGoogleFontCMake" )]]

project("DigitalBloom"
    VERSION "27.10"
    DESCRIPTION "Digital Bloom Application for Digital Bloom Programme"
    LANGUAGES CXX C
)

if(ANDROID)
    include(${CMAKE_SOURCE_DIR}/cmake/definesslandroid.cmake)
endif()


if(WIN32)
    include(${CMAKE_SOURCE_DIR}/cmake/definesslwindll.cmake)
endif()


set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(APP_NAME ${CMAKE_PROJECT_NAME})
set(APP_VERSION ${CMAKE_PROJECT_VERSION})

set(USE_ZXINGCPP ON CACHE BOOL "Force my bool variable to OFF" FORCE)
set(USE_ORM ON CACHE BOOL "Force my bool variable to OFF" FORCE)
set(USE_BOOST ON CACHE BOOL "Force my bool variable to OFF" FORCE)
set(USE_HARU ON CACHE BOOL "Force my bool variable to OFF" FORCE)
set(USE_WT ON CACHE BOOL "Force my bool variable to OFF" FORCE)
set(USE_ZL ON CACHE BOOL "Force my bool variable to OFF" FORCE)
set(USE_CRYPTOPP ON CACHE BOOL "Force my bool variable to OFF" FORCE)
set(USE_JSONCPP ON CACHE BOOL "Force my bool variable to OFF" FORCE)
set(USE_JSONCONS ON CACHE BOOL "Force my bool variable to OFF" FORCE)


# Define where Boost will be built and where the headers are
set(BOOST_ROOT "${CMAKE_BINARY_DIR}/boost" CACHE PATH "Root directory for built Boost" FORCE)
# Set the include path in the cache so Wt can pick it up
set(BOOST_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/boost" CACHE PATH "Boost include directory" FORCE)
# Set this to use Wt's proprietary method for finding Boost (optional, but robust)
#set(BOOST_VERSION "1_89" CACHE STRING "Boost version, e.g. 1_85" FORCE) # Replace with your actual version

# Set build options for Boost (e.g., static libs, multi-threaded)
set(Boost_USE_STATIC_LIBS ON CACHE BOOL "Use static Boost libraries" FORCE)
set(Boost_USE_MULTITHREADED OFF CACHE BOOL "Use multi-threaded Boost libraries" FORCE)
set(Boost_NO_SYSTEM_PATHS TRUE CACHE BOOL "Tell FindBoost not to look in system paths" FORCE) # Crucial

# set(AVCODEC_INCLUDE_DIR  "${CMAKE_SOURCE_DIR}/ffmpeg-kit/prebuilt/android-arm/ffmpeg/include" CACHE STRING "avcodec" FORCE)
# set(AVCODEC_LIBRARY  "${CMAKE_SOURCE_DIR}/ffmpeg-kit/prebuilt/android-arm/ffmpeg/lib/libavcodec.so" CACHE STRING "avcodec" FORCE)
# set(AVUTIL_INCLUDE_DIR  "${CMAKE_SOURCE_DIR}/ffmpeg-kit/prebuilt/android-arm/ffmpeg/include" CACHE STRING "avutil" FORCE)
# set(AVUTIL_LIBRARY  "${CMAKE_SOURCE_DIR}/ffmpeg-kit/prebuilt/android-arm/ffmpeg/lib/libavutil.so" CACHE STRING "avutil" FORCE)
# set(OPENSSL_INCLUDE_DIR  "${CMAKE_SOURCE_DIR}/ffmpeg-kit/prebuilt/android-arm/ffmpeg/include" CACHE STRING "openssl" FORCE)
# set(OPENSSL_CRYPTO_LIBRARY  "${CMAKE_SOURCE_DIR}/opensslandroid/armeabi-v7a/lib/libcrypto.so" CACHE STRING "openssl" FORCE)
# set(OPENSSL_SSL_LIBRARY  "${CMAKE_SOURCE_DIR}/opensslandroid/armeabi-v7a/lib/libssl.a" CACHE STRING "openssl" FORCE)



option(USE_QZXING "Build with QZXing support (decoder&encoder)" OFF)
option(USE_ZXINGCPP "Build with zxing-cpp support (decoder&encoder)" ON)
option(USE_ZINT "Build with zint support (encoder)" OFF)

################################################################################

message(STATUS "[CMAKE]   CMAKE_VERSION             = ${CMAKE_VERSION}")
message(STATUS "[CMAKE]   CMAKE_PREFIX_PATH         = ${CMAKE_PREFIX_PATH}")
message(STATUS "[SYSTEM]  (HOST) SYSTEM NAME        = ${CMAKE_HOST_SYSTEM_NAME}")
message(STATUS "[SYSTEM]  (HOST) SYSTEM VERSION     = ${CMAKE_HOST_SYSTEM_VERSION}")
message(STATUS "[SYSTEM]  (TARGET) SYSTEM NAME      = ${CMAKE_SYSTEM_NAME}")
message(STATUS "[SYSTEM]  (TARGET) SYSTEM VERSION   = ${CMAKE_SYSTEM_VERSION}")
message(STATUS "[PROJECT] SOURCE_DIR                = ${CMAKE_SOURCE_DIR}")
message(STATUS "[PROJECT] BUILD_TYPE                = ${CMAKE_BUILD_TYPE}")
message(STATUS "[PROJECT] TOOLCHAIN_FILE            = ${CMAKE_TOOLCHAIN_FILE}")
message(STATUS "[PROJECT] QT_HOST_PATH              = ${QT_HOST_PATH}")

################################################################################

find_package(Qt6 REQUIRED COMPONENTS
  Core
  CorePrivate
  Gui
  GuiPrivate
  Qml
  Quick
  QuickControls2
  Svg
  Xml
  Quick3D
  Core5Compat
  Network
  Sql
  Positioning
  Test
  Multimedia
  Concurrent
  Bluetooth
  LabsQmlModels
  Charts
)

#[[find_package(Qt6 REQUIRED COMPONENTS Core Bluetooth Positioning Network Sql)
find_package(Qt6 REQUIRED COMPONENTS Gui Svg Qml Quick QuickControls2 LabsQmlModels Widgets Charts)
find_package(Qt6 REQUIRED COMPONENTS LinguistTools)

qt_standard_project_setup(REQUIRES 6.7
                          I18N_SOURCE_LANGUAGE en
                          I18N_TRANSLATED_LANGUAGES ca da de es fr fy hu nb nl nn pl pt ru zh_CN zh_TW)]]

qt_policy(SET QTP0001 NEW)
qt_policy(SET QTP0002 NEW)
qt_policy(SET QTP0003 NEW)



set(MATERIALDESIGNICONS_ICONS_DIR "${CMAKE_BINARY_DIR}/eric/materialdesignicons-src/svg" CACHE INTERNAL "MaterialDesignIcons" )


if(WIN32)
  file(COPY ${CMAKE_SOURCE_DIR}/dep/eric
     DESTINATION ${CMAKE_BINARY_DIR})
  file(COPY ${CMAKE_SOURCE_DIR}/dep/CPM_modules
     DESTINATION ${CMAKE_BINARY_DIR})
else()
  file(COPY ${CMAKE_SOURCE_DIR}/dep/eric
     DESTINATION ${CMAKE_BINARY_DIR})
 file(COPY ${CMAKE_SOURCE_DIR}/dep/CPM_modules
     DESTINATION ${CMAKE_BINARY_DIR})
endif()

#[[file(COPY ${CMAKE_SOURCE_DIR}/fonts/res/qml
     DESTINATION ${CMAKE_BINARY_DIR})]]
set(QATERIAL_USE_LOCAL_CPM_FILE ON)

qt_standard_project_setup(REQUIRES 6.8)

# Download & Configure Qaterial as part of your build system
include(FetchContent)
set(FETCHCONTENT_QUIET off)
set(FETCHCONTENT_FULLY_DISCONNECTED on)

get_filename_component(fc_base "eric"
                       REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")
set(FETCHCONTENT_BASE_DIR ${fc_base})
FetchContent_Declare(
  Qaterial
  GIT_REPOSITORY https://github.com/OlivierLDff/Qaterial.git
  GIT_TAG        master
  GIT_SHALLOW    1
)

if(NOT Qaterial_POPULATED)
  FetchContent_MakeAvailable(Qaterial)
endif()

if(WIN32)
  target_compile_options(${QATERIAL_TARGET} PRIVATE
      $<$<CXX_COMPILER_ID:MSVC>:/constexpr:steps>
      $<$<CXX_COMPILER_ID:MSVC>:2000000>
      $<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus /permissive->
  )

# if(MSVC OR WIN32)
#   # make sure it's a cache variable so re-config works nicely
#   set(CMAKE_RC_COMPILER_LAUNCHER "" CACHE STRING "Disable compiler launcher for RC")
# endif()

else()
    # This block executes if the target system is not Windows
    message(STATUS "Not targeting Windows.")
    # Add configurations for other platforms (Linux, macOS, etc.)
endif()

if(ANDROID)
   # set(ANDROID_PLATFORM android-28 CACHE STRING "Minimum Android API level." FORCE)
endif()

if (USE_ZXINGCPP)
    # build zxing-cpp
    add_subdirectory(thirdparty/zxing-cpp)
    # build zxing-cpp Qt wrapper
    add_subdirectory(thirdparty/zxing-cpp/wrappers/qt)
    set(zxingcpp true)
    add_definitions(-Dzxingcpp)
elseif (USE_QZXING)
    # build QZxing
    #add_subdirectory(thirdparty/QZXing)
    #target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE QZXing)
    set(QZXING_USE_QML ON)
    set(QZXING_USE_ENCODER ON)
    set(QZXING_MULTIMEDIA ON)
    add_subdirectory(qzxing/src)
    set(qzxing true)
    add_definitions(-Dqzxing)
else()
endif()

if (USE_ZINT)
    # build zint
    add_subdirectory(thirdparty/zint-qml)
    # build ZintQml
    add_subdirectory(thirdparty/zint-qml/wrappers/qml)
    set(zint true)
    add_definitions(-Dzint)
endif()

if (USE_WT)
    add_subdirectory(${CMAKE_SOURCE_DIR}/wt)
endif()


if (USE_JSONCPP)
    add_subdirectory(${CMAKE_SOURCE_DIR}/jsoncpp)
endif()

if (USE_JSONCONS)
    add_subdirectory(${CMAKE_SOURCE_DIR}/jsoncons)
endif()

if (USE_ZL)
    add_subdirectory(${CMAKE_SOURCE_DIR}/mystreamserver)
endif()

if (USE_CRYPTOPP)
    add_subdirectory(${CMAKE_SOURCE_DIR}/cryptopp-cmake)
endif()


if (USE_ORM)
   # add_definitions(-D_QX_ENABLE_QT_GUI -D_QX_ENABLE_QT_NETWORK)
   set(_QX_ENABLE_QT_GUI ON CACHE BOOL "Force my bool variable to OFF" FORCE)
   set(_QX_ENABLE_QT_NETWORK ON CACHE BOOL "Force my bool variable to OFF" FORCE)
   set(_QX_STATIC_BUILD ON CACHE BOOL "Force my bool variable to OFF" FORCE)
    add_subdirectory(${CMAKE_SOURCE_DIR}/wt/QxOrm)
endif()


add_subdirectory(qsyncable)
add_subdirectory(qtstatusbar)
add_subdirectory(QuickDownload)
add_subdirectory(SortFilterProxyModel)
add_subdirectory(jwt)

# SingleApplication

add_subdirectory(thirdparty/SingleApplication)

# AppUtils
set(UTILS_CLIPBOARD true)
set(UTILS_DOCK_ENABLED true)
add_subdirectory(thirdparty/AppUtils)

# MobileUI
add_subdirectory(thirdparty/MobileUI)

# MobileSharing
add_subdirectory(thirdparty/MobileSharing)


# SunAndMoon
add_subdirectory(thirdparty/SunAndMoon)


# ComponentLibrary
add_subdirectory(thirdparty/ComponentLibrary)


set(SOURCES
    iconcategorymodel.cpp iconcategorymodel.h
    iconnamemodel.cpp iconnamemodel.h
    ${CMAKE_SOURCE_DIR}/cpp/CopyAssets.h
    ${CMAKE_SOURCE_DIR}/cpp/EricImage.hpp
    ${CMAKE_SOURCE_DIR}/cpp/EricType.hpp
    ${CMAKE_SOURCE_DIR}/cpp/MarkerModel.hpp
    ${CMAKE_SOURCE_DIR}/cpp/FileHelper.hpp
    ${CMAKE_SOURCE_DIR}/cpp/JsonTools.hpp
    ${CMAKE_SOURCE_DIR}/cpp/JsonUtils.h
    ${CMAKE_SOURCE_DIR}/cpp/Macros.h
    ${CMAKE_SOURCE_DIR}/cpp/MyTimer.hpp
    ${CMAKE_SOURCE_DIR}/cpp/Net/anacache.h
    ${CMAKE_SOURCE_DIR}/cpp/Net/configi.h
    ${CMAKE_SOURCE_DIR}/cpp/Net/fileio.h
    ${CMAKE_SOURCE_DIR}/cpp/Net/gu.h
    ${CMAKE_SOURCE_DIR}/cpp/Net/guiapplication.h
    ${CMAKE_SOURCE_DIR}/cpp/tools/NotificationManager.h
    ${CMAKE_SOURCE_DIR}/cpp/tools/DatabaseManager.h
    ${CMAKE_SOURCE_DIR}/cpp/tools/SettingsManager.h
    ${CMAKE_SOURCE_DIR}/cpp/tools/Barcode.h
    ${CMAKE_SOURCE_DIR}/cpp/tools/BarcodeManager.h
    ${CMAKE_SOURCE_DIR}/cpp/tools/utils_camera.h
    ${CMAKE_SOURCE_DIR}/cpp/Net/imageloader.hpp
    ${CMAKE_SOURCE_DIR}/cpp/Net/include.h
    ${CMAKE_SOURCE_DIR}/cpp/Net/mimetypes.h
    ${CMAKE_SOURCE_DIR}/cpp/Net/paginationoptions.h
    ${CMAKE_SOURCE_DIR}/cpp/Net/rest.h
    ${CMAKE_SOURCE_DIR}/cpp/Net/restconsumer.h
    ${CMAKE_SOURCE_DIR}/cpp/Net/restconsumer2.h
    ${CMAKE_SOURCE_DIR}/cpp/Net/restconsumer3.h
    ${CMAKE_SOURCE_DIR}/cpp/Net/restmodel.h
    ${CMAKE_SOURCE_DIR}/cpp/Net/restuploader.h
    ${CMAKE_SOURCE_DIR}/cpp/Net/stringlistmodel.h
    ${CMAKE_SOURCE_DIR}/cpp/VariantHelpers.hpp
    ${CMAKE_SOURCE_DIR}/cpp/eric_data.h
    ${CMAKE_SOURCE_DIR}/cpp/sqliteone.hpp
    ${CMAKE_SOURCE_DIR}/cpp/export.h
    ${CMAKE_SOURCE_DIR}/cpp/precompiled.h
    ${CMAKE_SOURCE_DIR}/cpp/Parser/Exception.h
    ${CMAKE_SOURCE_DIR}/cpp/Parser/Field.h
    ${CMAKE_SOURCE_DIR}/cpp/Parser/Parser.h
    ${CMAKE_SOURCE_DIR}/cpp/base/Carousel.h
    ${CMAKE_SOURCE_DIR}/cpp/base/CsvUtil.h
    ${CMAKE_SOURCE_DIR}/cpp/base/DataNet.h
    ${CMAKE_SOURCE_DIR}/cpp/base/DeferredWidget.h
    ${CMAKE_SOURCE_DIR}/cpp/base/EricButton.h
    ${CMAKE_SOURCE_DIR}/cpp/base/MapperSoftware.h
    ${CMAKE_SOURCE_DIR}/cpp/base/ServerResource.h
    ${CMAKE_SOURCE_DIR}/cpp/base/UserNameResource.h
    ${CMAKE_SOURCE_DIR}/cpp/components/DataModels.h
    ${CMAKE_SOURCE_DIR}/cpp/components/DataModels.h
    ${CMAKE_SOURCE_DIR}/cpp/ericsettings/Macros.h
    ${CMAKE_SOURCE_DIR}/cpp/ericsettings/SettingsManagerEric.h
    ${CMAKE_SOURCE_DIR}/cpp/ericsettings/SqliteManager.h
    ${CMAKE_SOURCE_DIR}/cpp/views/AppLinks.h
    ${CMAKE_SOURCE_DIR}/cpp/views/AppRoles.h
    ${CMAKE_SOURCE_DIR}/cpp/views/MainTemplate.h
    ${CMAKE_SOURCE_DIR}/cpp/views/Module.h
    ${CMAKE_SOURCE_DIR}/cpp/views/Software.h
    ${CMAKE_SOURCE_DIR}/cpp/Parser/Exception.cc
    ${CMAKE_SOURCE_DIR}/cpp/Parser/Field.cc
    ${CMAKE_SOURCE_DIR}/cpp/Parser/Parser.cc
    ${CMAKE_SOURCE_DIR}/cpp/base/CsvUtil.C
    ${CMAKE_SOURCE_DIR}/cpp/base/DataNet.cpp
    ${CMAKE_SOURCE_DIR}/cpp/components/AnchorAllItems.cpp
    ${CMAKE_SOURCE_DIR}/cpp/components/AnchorNewItem.cpp
    ${CMAKE_SOURCE_DIR}/cpp/components/AnchorRegister.cpp
    ${CMAKE_SOURCE_DIR}/cpp/components/AnchorRetrieve.cpp
    ${CMAKE_SOURCE_DIR}/cpp/components/AnchorSetUp.cpp
    ${CMAKE_SOURCE_DIR}/cpp/components/AnchorUpdateItems.cpp
    ${CMAKE_SOURCE_DIR}/cpp/components/DataModels.cpp
    ${CMAKE_SOURCE_DIR}/cpp/components/LeafletMap.cpp
    ${CMAKE_SOURCE_DIR}/cpp/components/VirtualModel.cpp
    ${CMAKE_SOURCE_DIR}/cpp/orderserver/servercontrollerwt.h
    ${CMAKE_SOURCE_DIR}/cpp/orderserver/servercontrollerwt.cpp
    ${CMAKE_SOURCE_DIR}/cpp/orderserver/AssetExtractor.h
    ${CMAKE_SOURCE_DIR}/cpp/orderserver/AssetExtractor.cpp
    ${CMAKE_SOURCE_DIR}/cpp/orderserver/LibArchiveExtractor.h
    ${CMAKE_SOURCE_DIR}/cpp/orderserver/LibArchiveExtractor.cpp
    ${CMAKE_SOURCE_DIR}/cpp/orderserver/LibArchiveCompressor.h
    ${CMAKE_SOURCE_DIR}/cpp/orderserver/LibArchiveCompressor.cpp
    ${CMAKE_SOURCE_DIR}/cpp/orderserver/LibArchiveQmlBridge.h
    ${CMAKE_SOURCE_DIR}/cpp/orderserver/LibArchiveQmlBridge.cpp
    ${CMAKE_SOURCE_DIR}/cpp/orderserver/DataUploader.h
    ${CMAKE_SOURCE_DIR}/cpp/orderserver/DataUploader.cpp
    ${CMAKE_SOURCE_DIR}/cpp/orderserver/WrapUpload.h
    ${CMAKE_SOURCE_DIR}/cpp/orderserver/WrapUpload.cpp
    ${CMAKE_SOURCE_DIR}/cpp/orderserver/QmlBackend.h
    ${CMAKE_SOURCE_DIR}/cpp/orderserver/QmlBackend.cpp
    ${CMAKE_SOURCE_DIR}/cpp/orderserver/TimedUploader.h
    ${CMAKE_SOURCE_DIR}/cpp/orderserver/TimedUploader.cpp
    ${CMAKE_SOURCE_DIR}/cpp/orderserver/Android.h
    ${CMAKE_SOURCE_DIR}/cpp/orderserver/Android.C
    ${CMAKE_SOURCE_DIR}/cpp/ormserver/QxHttpServerController.h
    ${CMAKE_SOURCE_DIR}/cpp/streamserver/StreamServer.hpp
    ${CMAKE_SOURCE_DIR}/cpp/streamserver/StreamServer.cpp
    ${CMAKE_SOURCE_DIR}/cpp/ormserver/QxHttpServerController.cpp
    ${CMAKE_SOURCE_DIR}/cpp/orm/categories.hpp
    ${CMAKE_SOURCE_DIR}/cpp/orm/cryptocenter.hpp
    ${CMAKE_SOURCE_DIR}/cpp/orm/ids.hpp
    ${CMAKE_SOURCE_DIR}/cpp/orm/items.hpp
    ${CMAKE_SOURCE_DIR}/cpp/orm/metrics.hpp
    ${CMAKE_SOURCE_DIR}/cpp/orm/mysecrets.h
    ${CMAKE_SOURCE_DIR}/cpp/orm/orders.h
    ${CMAKE_SOURCE_DIR}/cpp/orm/locations.h
    ${CMAKE_SOURCE_DIR}/cpp/orm/delivery.h
    ${CMAKE_SOURCE_DIR}/cpp/orm/logistics.h
    ${CMAKE_SOURCE_DIR}/cpp/orm/categories.cpp
    ${CMAKE_SOURCE_DIR}/cpp/orm/cryptocenter.cpp
    ${CMAKE_SOURCE_DIR}/cpp/orm/ids.cpp
    ${CMAKE_SOURCE_DIR}/cpp/orm/items.cpp
    ${CMAKE_SOURCE_DIR}/cpp/orm/metrics.cpp
    ${CMAKE_SOURCE_DIR}/cpp/orm/mysecrets.cpp
    ${CMAKE_SOURCE_DIR}/cpp/orm/orders.cpp
    ${CMAKE_SOURCE_DIR}/cpp/orm/locations.cpp
    ${CMAKE_SOURCE_DIR}/cpp/orm/delivery.cpp
    ${CMAKE_SOURCE_DIR}/cpp/orm/logistics.cpp
    ${CMAKE_SOURCE_DIR}/cpp/fields.h
    ${CMAKE_SOURCE_DIR}/cpp/fieldsform.h
    ${CMAKE_SOURCE_DIR}/cpp/json.hpp
    ${CMAKE_SOURCE_DIR}/cpp/sms/sms.h
    ${CMAKE_SOURCE_DIR}/cpp/system.h
    ${CMAKE_SOURCE_DIR}/cpp/translationsmanager.h
    ${CMAKE_SOURCE_DIR}/cpp/uiconstants.hpp
    ${CMAKE_SOURCE_DIR}/cpp/unsafearea.hpp
    ${CMAKE_SOURCE_DIR}/cpp/FileHelper.cpp
    ${CMAKE_SOURCE_DIR}/cpp/JsonUtils.cpp
    ${CMAKE_SOURCE_DIR}/cpp/MyJsonTools.cpp
    ${CMAKE_SOURCE_DIR}/cpp/MyJsonTools.hpp
    ${CMAKE_SOURCE_DIR}/cpp/Net/anacache.cpp
    ${CMAKE_SOURCE_DIR}/cpp/Net/configi.cpp
    ${CMAKE_SOURCE_DIR}/cpp/Net/fileio.cpp
    ${CMAKE_SOURCE_DIR}/cpp/Net/guiapplication.cpp
    ${CMAKE_SOURCE_DIR}/cpp/Net/imageloader.cpp
    ${CMAKE_SOURCE_DIR}/cpp/Net/rest.cpp
    ${CMAKE_SOURCE_DIR}/cpp/Net/restconsumer.cpp
    ${CMAKE_SOURCE_DIR}/cpp/Net/restconsumer2.cpp
    ${CMAKE_SOURCE_DIR}/cpp/Net/restconsumer3.cpp
    ${CMAKE_SOURCE_DIR}/cpp/Net/restmodel.cpp
    ${CMAKE_SOURCE_DIR}/cpp/Net/restuploader.cpp
    ${CMAKE_SOURCE_DIR}/cpp/Net/stringlistmodel.cpp
    ${CMAKE_SOURCE_DIR}/cpp/ericsettings/SettingsManagerEric.cpp
    ${CMAKE_SOURCE_DIR}/cpp/ericsettings/SqliteManager.cpp
    ${CMAKE_SOURCE_DIR}/cpp/fieldsform.cpp
    ${CMAKE_SOURCE_DIR}/cpp/sms/sms.cpp
    ${CMAKE_SOURCE_DIR}/cpp/system.cpp
    ${CMAKE_SOURCE_DIR}/cpp/translationsmanager.cpp
    ${CMAKE_SOURCE_DIR}/cpp/unsafearea.cpp
    ${CMAKE_SOURCE_DIR}/cpp/EricType.cpp
    ${CMAKE_SOURCE_DIR}/cpp/MarkerModel.cpp
    ${CMAKE_SOURCE_DIR}/cpp/tools/NotificationManager.cpp
    ${CMAKE_SOURCE_DIR}/cpp/tools/DatabaseManager.cpp
    ${CMAKE_SOURCE_DIR}/cpp/tools/SettingsManager.cpp
    ${CMAKE_SOURCE_DIR}/cpp/tools/Barcode.cpp
    ${CMAKE_SOURCE_DIR}/cpp/tools/BarcodeManager.cpp
    ${CMAKE_SOURCE_DIR}/cpp/tools/utils_camera.cpp
    ${CMAKE_SOURCE_DIR}/cpp/tools/ServerManager.cpp
    ${CMAKE_SOURCE_DIR}/cpp/tools/ServerManager.h
    main.cpp
    thirdparty/RC4/rc4.cpp thirdparty/RC4/rc4.h
    thirdparty/IconLibrary/IconLibrary_material.qrc
    thirdparty/IconLibrary/IconLibrary_lucide.qrc
    resources.qrc
    data-assets.qrc
    assets/myassets.qrc
    myserverassets.qrc
    qml/qml.qrc
   # qml/ui.qrc
)

if(ANDROID)
    set(SOURCES_ANDROID
        cpp/tools/AndroidService.cpp cpp/tools/AndroidService.h
    )
endif()

if(NOT IOS AND NOT ANDROID)
    # Set app icons
    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
       # set(app_icon_windows "${CMAKE_SOURCE_DIR}/assets/windows/${CMAKE_PROJECT_NAME}.rc")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        set(app_icon_macos "${CMAKE_SOURCE_DIR}/assets/macos/${CMAKE_PROJECT_NAME}.icns")
    endif()

    set(SOURCES_DESKTOP
        ${CMAKE_SOURCE_DIR}/cpp/tools/SystrayManager.cpp ${CMAKE_SOURCE_DIR}/cpp/tools/SystrayManager.h
        ${app_icon_windows}
        ${app_icon_macos}
    )
endif()

if (IOS OR ANDROID)
    # Mobile
    set(QAPPLICATION_CLASS QGuiApplication)
else()
    # Desktop
    set(QAPPLICATION_CLASS QApplication)
endif()

# # 1. Use qt_add_big_resources() to generate object files from the QRC.
# #    This command populates the variable BIG_RESOURCE_OBJECTS with those object file paths.
#qt_add_big_resources(BIG_RESOURCE_OBJECTS "${CMAKE_SOURCE_DIR}/myserverassets.qrc")

# # 2. Add the original .qrc file to the 'SOURCES' list just for Qt Creator's display purposes.
# #    It does not get compiled as a source file; it is just listed for convenience.
#list(APPEND SOURCES "${CMAKE_SOURCE_DIR}/myserverassets.qrc")

# # 3. Ensure this specific .qrc file is explicitly ignored by any other AUTORCC processes.
#set_property(SOURCE "${CMAKE_SOURCE_DIR}/myserverassets.qrc" PROPERTY SKIP_AUTORCC ON)

qt_add_executable(${CMAKE_PROJECT_NAME}
    ${SOURCES}
   # ${BIG_RESOURCE_OBJECTS}
    ${SOURCES_ANDROID}
    ${SOURCES_DESKTOP}
)

if(WIN32)
    set(RC_IN  "${CMAKE_SOURCE_DIR}/assets/windows/${CMAKE_PROJECT_NAME}.rc")
    set(RC_RES "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.res")

    add_custom_command(
        OUTPUT "${RC_RES}"
        COMMAND "${CMAKE_RC_COMPILER}" /fo "${RC_RES}" "${RC_IN}"
        MAIN_DEPENDENCY "${RC_IN}"
        COMMENT "Compiling resource ${RC_IN} -> ${RC_RES}"
        VERBATIM
    )

    # Tell CMake this file is generated (so it won't expect it to exist at configure time)
    set_source_files_properties("${RC_RES}" PROPERTIES GENERATED TRUE)

    # Make a custom target that depends on the generated .res
    add_custom_target(compile_rc DEPENDS "${RC_RES}")

    # Ensure the exe depends on the custom target (so the .res is built before link)
    add_dependencies(${CMAKE_PROJECT_NAME} compile_rc)

    # Make sure the .res is treated as a source for the target (so CMake adds it to the link line)
    # Do both: add as a source and also add it to the link input (some generators need the latter).
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE "${RC_RES}")
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE "${RC_RES}")
endif()


qt_add_ios_ffmpeg_libraries(${CMAKE_PROJECT_NAME})

qt_add_qml_module(${CMAKE_PROJECT_NAME}
    URI ${CMAKE_PROJECT_NAME}
    QML_FILES
        qml/ui/MobileApplication.qml
        qml/ui/MobileHeader.qml
        qml/ui/MobileDrawer.qml
        qml/ui/MobilePermissions.qml
        qml/ui/DesktopApplication.qml
        qml/ui/DesktopSidebar.qml

        qml/ui/ScreenTutorial.qml
        qml/ui/ScreenMainMenu.qml
        qml/ui/ScreenSettings.qml
        qml/ui/ScreenAbout.qml
        qml/ui/ScreenAboutFormats.qml

        qml/ui/ScreenBarcodeReader.qml
        qml/ui/ScreenBarcodeHistory.qml
        qml/ui/ScreenBarcodeDetails.qml
        qml/ui/ScreenBarcodeWriter.qml
        qml/ui/Reader_QZXing.qml
        qml/ui/Reader_ZXingCpp.qml
        qml/ui/Writer_QZXing.qml
        qml/ui/Writer_ZXingCpp.qml
        qml/ui/Writer_Zint.qml

        qml/ui/popups/PopupBarcodeFullscreen.qml
        qml/ui/popups/PopupHistoryDelete.qml

        qml/ui/components/MenuDebug.qml
        qml/ui/components/MenuCamera.qml
        qml/ui/components/MenuFormats.qml
        qml/ui/components/MenuScreens.qml
        qml/ui/components/WidgetBarcodeHistory.qml
        qml/ui/components/WidgetBarcodeResult.qml
)

#[[ qt_add_translations(${CMAKE_PROJECT_NAME}
TS_FILE_BASE ${CMAKE_PROJECT_NAME}
TS_FILE_DIR ${CMAKE_SOURCE_DIR}/i18n/
RESOURCE_PREFIX "/i18n"
)]]

target_precompile_headers(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/cpp/MyJsonTools.hpp )

set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
        QT_QML_IMPORT_PATH "${CMAKE_SOURCE_DIR}/qml"
    )

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    QT_NO_KEYWORDS
    _BUILDING_QX_DIGITALBLOOMORDERS
    LIBARCHIVE_STATIC
    _QX_ENABLE_QT_NETWORK
    _QX_STATIC_BUILD
)

set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
  AUTOMOC ON
  AUTORCC ON    # optional if you use .qrc
 # AUTOUIC ON    # optional if you use .ui
)

set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

target_compile_features(${CMAKE_PROJECT_NAME} PRIVATE cxx_std_17)


# add_definitions(-UZLIB_WINAPI)     # undefine if it was set anywhere
# add_definitions(-DZLIB_WINAPI=0)   # explicitly force it off

# target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
#     -UZLIB_WINAPI  # undefine if set elsewhere
#     ZLIB_WINAPI=0
# )

# target_compile_definitions(DigitalBloom PRIVATE ZLIB_WINAPI=0)
# target_compile_options(zlib PRIVATE "-UZLIB_WINAPI")
# set(ZLIB_WINAPI OFF CACHE BOOL "Do not use ZLIB WIN API" FORCE)
# add_definitions(-DZLIB_WINAPI=0)

list(APPEND QML_IMPORT_PATH "${CMAKE_SOURCE_DIR}/material")
list(APPEND QML_IMPORT_PATH "${CMAKE_SOURCE_DIR}/qml/assets")
set(QML_IMPORT_PATH "${QML_IMPORT_PATH}" CACHE STRING "Qt Creator extra QML import paths" FORCE)

# ---------------------------
# Collect link libraries once
# ---------------------------
# start lists

set(_MY_LINK_PRIVATE
    Qt6::Core
    Qt6::CorePrivate
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Core5Compat
    Qt6::Sql
    Qt6::Quick3D
    Qt6::Network
    Qt6::Positioning
    Qt6::Test
    Qt6::GuiPrivate
    Qt6::Concurrent
    Qt6::Multimedia
    Qt6::Bluetooth
    Qt6::Svg
    Qt6::Charts
    Qaterial
)

set(_MY_LINK_PUBLIC)  # empty initially, append conditionally where PUBLIC is required

# Conditionals that append to the lists instead of calling target_link_libraries repeatedly

if (USE_ZXINGCPP)
    list(APPEND _MY_LINK_PRIVATE ZXing ZXingQt)
elseif (USE_QZXING)
    list(APPEND _MY_LINK_PRIVATE qzxing)
endif()

if (USE_ZINT)
    list(APPEND _MY_LINK_PRIVATE zint ZintQml)
endif()

if (USE_WT)
    if (UNIX)
        list(APPEND _MY_LINK_PUBLIC wtdbo wthttp wt ${BOOST_FS_LIB} ${BOOST_SYSTEM_LIB})
        # if you want the lzma/zstd/z libs on UNIX uncomment and append them instead:
        #list(APPEND _MY_LINK_PUBLIC liblzma libzstd_static zlibstatic)
    else()
        list(APPEND _MY_LINK_PUBLIC wtdbo wthttp wt ${BOOST_FS_LIB} ${BOOST_SYSTEM_LIB})
        # for Windows alternative libs:
        #list(APPEND _MY_LINK_PUBLIC liblzma libzstd_static zlibstatic)
    endif()

    # add WT include dir (PUBLIC because consumers may need it)
    list(APPEND _MY_INCLUDES_PUBLIC ${CMAKE_SOURCE_DIR}/wt/src)

    # append many boost components (kept as PRIVATE because originally they were added PRIVATE)
    list(APPEND _MY_LINK_PRIVATE
      boost_accumulators
      boost_algorithm
      boost_align
      boost_any
      boost_array
      boost_asio
      boost_assert
      boost_assign
      boost_atomic
      boost_beast
      boost_bimap
      boost_bind
      boost_bloom
      boost_callable_traits
      boost_charconv
      boost_chrono
      boost_circular_buffer
      boost_cobalt
      boost_compat
      boost_compute
      boost_concept_check
      boost_config
      boost_container
      boost_container_hash
      boost_context
      boost_contract
      boost_conversion
      boost_convert
      boost_core
      boost_coroutine
      boost_coroutine2
      boost_crc
      boost_date_time
      boost_describe
      boost_detail
      boost_dll
      boost_dynamic_bitset
      boost_endian
      boost_exception
      boost_fiber
      boost_filesystem
      boost_flyweight
      boost_foreach
      boost_format
      boost_function
      boost_function_types
      boost_functional
      boost_fusion
      boost_geometry
      boost_gil
      boost_graph
      boost_hana
      boost_hash2
      boost_headers
      boost_heap
      boost_histogram
      boost_hof
      boost_icl
      boost_integer
      boost_interprocess
      boost_intrusive
      boost_io
      boost_iostreams
      boost_iterator
      boost_json
      boost_lambda
      boost_lambda2
      boost_leaf
      boost_lexical_cast
      boost_locale
      boost_lockfree
      boost_log
      boost_logic
      boost_math
      boost_metaparse
      boost_move
      boost_mp11
      boost_mpl
      boost_mqtt5
      boost_msm
      boost_multi_array
      boost_multi_index
      boost_multiprecision
      boost_nowide
      boost_openmethod
      boost_optional
      boost_outcome
      boost_parameter
      boost_parser
      boost_pfr
      boost_phoenix
      boost_poly_collection
      boost_polygon
      boost_pool
      boost_predef
      boost_preprocessor
      boost_process
      boost_program_options
      boost_property_map
      boost_property_tree
      boost_proto
      boost_ptr_container
      boost_qvm
      boost_random
      boost_range
      boost_ratio
      boost_rational
      boost_redis
      boost_regex
      boost_safe_numerics
      boost_scope
      boost_scope_exit
      boost_serialization
      boost_signals2
      boost_smart_ptr
      boost_sort
      boost_spirit
      boost_stacktrace
      boost_statechart
      boost_static_assert
      boost_static_string
      boost_stl_interfaces
      boost_system
      boost_unit_test_framework
      boost_prg_exec_monitor
      boost_test_exec_monitor
      boost_thread
      boost_throw_exception
      boost_timer
      boost_tokenizer
      boost_tti
      boost_tuple
      boost_type_erasure
      boost_type_index
      boost_type_traits
      boost_typeof
      boost_units
      boost_unordered
      boost_url
      boost_utility
      boost_uuid
      boost_variant
      boost_variant2
      boost_vmd
      boost_wave
      boost_winapi
      boost_xpressive
      boost_yap
      archive_static
    )
endif()

if (USE_JSONCPP)
    list(APPEND _MY_INCLUDES_PUBLIC ${CMAKE_SOURCE_DIR}/jsoncpp/include)
    list(APPEND _MY_LINK_PUBLIC jsoncpp_static)
endif()

if (USE_JSONCONS)
    list(APPEND _MY_INCLUDES_PUBLIC
      ${JSONCONS_INCLUDE_DIR}
      ${JSONCONS_TESTS_DIR}
      ${JSONCONS_THIRD_PARTY_INCLUDE_DIR}
      ${CMAKE_SOURCE_DIR}/jsoncons/include)
endif()


if(USE_ZL)

list(APPEND _MY_INCLUDES_PUBLIC
        ${CMAKE_SOURCE_DIR}/mystreamserver/ZLMediaKit/3rdpart
        ${CMAKE_SOURCE_DIR}/mystreamserver/ZLMediaKit/3rdpart/media-server/libflv/include
        ${CMAKE_SOURCE_DIR}/mystreamserver/ZLMediaKit/3rdpart/media-server/libmov/include
        ${CMAKE_SOURCE_DIR}/mystreamserver/ZLMediaKit/3rdpart/media-server/libmpeg/include
        ${CMAKE_SOURCE_DIR}/mystreamserver/ZLMediaKit/3rdpart/ZLToolKit/src
        ${CMAKE_SOURCE_DIR}/mystreamserver/ZLMediaKit/src
        ${CMAKE_SOURCE_DIR}/mystreamserver/ZLMediaKit/ext-codec
        ${CMAKE_SOURCE_DIR}/mystreamserver/ZLMediaKit/srt
		${CMAKE_SOURCE_DIR}/mystreamserver
    )
	
    if(ANDROID)
        list(APPEND _MY_LINK_PRIVATE
            StreamServer srt flv mov mpeg zltoolkit ext-codec zlmediakit log
            ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY}
        )

    else()
        list(APPEND _MY_LINK_PRIVATE
            StreamServer srt flv mov mpeg zltoolkit ext-codec zlmediakit
            ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY}
        )
    endif()

    # Optional alternative:
    # list(APPEND _MY_LINK_PRIVATE srt webrtc flv mov mpeg zltoolkit ext-codec zlmediakit mk_api)

endif()

if (USE_CRYPTOPP)
    list(APPEND _MY_LINK_PUBLIC cryptopp)
endif()

if (USE_ORM)
    list(APPEND _MY_LINK_PUBLIC QxOrm)
    list(APPEND _MY_INCLUDES_PUBLIC ${CMAKE_SOURCE_DIR}/wt/QxOrm/include)
endif()

if (USE_HARU)
    # keep linking commented variants in case you want them, but append includes
    list(APPEND _MY_INCLUDES_PUBLIC
        ${CMAKE_SOURCE_DIR}/wt/libharu/include
        ${CMAKE_BINARY_DIR}/wt/libharu/include
    )
    # if you want to link hpdf/zlib into target, uncomment and append to _MY_LINK_PUBLIC/_MY_LINK_PRIVATE appropriately
    if (WIN32)
      list(APPEND _MY_LINK_PUBLIC hpdf zlibstatic liblzma libzstd_static)
    else()
      list(APPEND _MY_LINK_PUBLIC hpdf z liblzma libzstd_static)
    endif()
endif()

# Append the miscellaneous libs that were scattered through the file (make them PRIVATE)
list(APPEND _MY_LINK_PRIVATE
    QSyncable
    StatusBar
    QuickDownload
    SortFilterProxyModel
    QJWT
    SingleApplication
    AppUtils
    MobileUI
    MobileSharing
    SunAndMoon
    ComponentLibraryplugin
)

# Widgets (only for desktop)
if (NOT IOS AND NOT ANDROID)
    find_package(Qt6 REQUIRED COMPONENTS Widgets)
    list(APPEND _MY_LINK_PRIVATE Qt6::Widgets)
endif()

# Final include directories lists
# start include lists (some were already appended above)
set(_MY_INCLUDES_PRIVATE
    ${CMAKE_SOURCE_DIR}/wt/boost/libarchive/libarchive
    ${CMAKE_SOURCE_DIR}/cpp
    ${CMAKE_SOURCE_DIR}/thirdparty
    ${CMAKE_SOURCE_DIR}/QuickDownload/src
)
# ensure public includes variable exists
if (NOT DEFINED _MY_INCLUDES_PUBLIC)
    set(_MY_INCLUDES_PUBLIC)
endif()

# ---------------------------
# Single-target calls
# ---------------------------
# One call to target_include_directories
# PUBLIC headers first, PRIVATE next
if (_MY_INCLUDES_PUBLIC)
    target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${_MY_INCLUDES_PUBLIC})
endif()
if (_MY_INCLUDES_PRIVATE)
    target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${_MY_INCLUDES_PRIVATE})
endif()

# One call to target_link_libraries with aggregated PUBLIC and PRIVATE lists
# (Ordering of IMPORTED libs follows the append order above)
if (_MY_LINK_PUBLIC)
    if (_MY_LINK_PRIVATE)
        target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${_MY_LINK_PRIVATE} PUBLIC ${_MY_LINK_PUBLIC})
    else()
        target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC ${_MY_LINK_PUBLIC})
    endif()
else()
    if (_MY_LINK_PRIVATE)
        target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${_MY_LINK_PRIVATE})
    endif()
endif()

# Done.



################################################################################




## Android
if(CMAKE_SYSTEM_NAME STREQUAL "Android")
    set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
        # QT_ANDROID_ABIS "armeabi-v7a;arm64-v8a;x86;x86_64"
        # QT_ANDROID_BUILD_ALL_ABIS ON
        QT_ANDROID_MIN_SDK_VERSION 23
        QT_ANDROID_TARGET_SDK_VERSION 36
        QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_SOURCE_DIR}/assets/android/"
    )

    set(EXTRA_LIBS
        "${CMAKE_SOURCE_DIR}/opensslandroid/armeabi-v7a/lib/libcrypto_3.so"
        "${CMAKE_SOURCE_DIR}/opensslandroid/armeabi-v7a/lib/libssl_3.so"
        "${CMAKE_SOURCE_DIR}/opensslandroid/arm64-v8a/lib/libcrypto_3.so"
        "${CMAKE_SOURCE_DIR}/opensslandroid/arm64-v8a/lib/libssl_3.so"
        "${CMAKE_SOURCE_DIR}/opensslandroid/x86/lib/libcrypto_3.so"
        "${CMAKE_SOURCE_DIR}/opensslandroid/x86/lib/libssl_3.so"
        "${CMAKE_SOURCE_DIR}/opensslandroid/x86_64/lib/libcrypto_3.so"
        "${CMAKE_SOURCE_DIR}/opensslandroid/x86_64/lib/libssl_3.so"
    )

    set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
        QT_ANDROID_EXTRA_LIBS "${EXTRA_LIBS}"
    )

    # Patched QtConnectivity
    # target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE QT_CONNECTIVITY_PATCHED)

    # Android background service
    find_package(Qt6 REQUIRED COMPONENTS GuiPrivate)
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Qt::CorePrivate ${ANDROID_EXTRA_LIBS})
endif()


## macOS
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    #set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0")

    set(app_icon_macos "${CMAKE_SOURCE_DIR}/assets/macos/${CMAKE_PROJECT_NAME}.icns")
    set_source_files_properties(${app_icon_macos} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

    set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE ON
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/assets/macos/Info.plist"
        # New Xcode attributes (???)
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER           "org.stllimited.DigitalBloom"
        XCODE_ATTRIBUTE_PRODUCT_NAME                        ${CMAKE_PROJECT_NAME}
        XCODE_ATTRIBUTE_CURRENT_PROJECT_VERSION             ${CMAKE_PROJECT_VERSION}
        XCODE_ATTRIBUTE_MARKETING_VERSION                   ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS             "${CMAKE_SOURCE_DIR}/assets/macos/${CMAKE_PROJECT_NAME}.entitlements"
    )
endif()

## iOS
if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    set(app_icon_ios "${CMAKE_SOURCE_DIR}/assets/ios/Images.xcassets")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${app_icon_ios})
    set_source_files_properties(${app_icon_ios} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

    set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE ON
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/assets/ios/Info.plist"
        # Qt
        QT_IOS_LAUNCH_SCREEN "${CMAKE_SOURCE_DIR}/assets/ios/LaunchScreen.storyboard"
        # New Xcode attributes (???)
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER           "org.stllimited.DigitalBloom"
        XCODE_ATTRIBUTE_PRODUCT_NAME                        ${CMAKE_PROJECT_NAME}
        XCODE_ATTRIBUTE_CURRENT_PROJECT_VERSION             ${CMAKE_PROJECT_VERSION}
        XCODE_ATTRIBUTE_MARKETING_VERSION                   ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET          "16.0"
        XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY              "1,2"   # iPhone,iPad
        XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME  "AppIcon"
        XCODE_ATTRIBUTE_GCC_GENERATE_DEBUGGING_SYMBOLS      "YES"
    )
endif()

## Windows
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # set(app_icon_windows "${CMAKE_SOURCE_DIR}/assets/windows/${CMAKE_PROJECT_NAME}.rc")

    set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE ON
    )
endif()

## Release build? Set "no debug" macros
if(CMAKE_BUILD_TYPE STREQUAL Release)
    target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
        NDEBUG
        QT_NO_DEBUG
        QT_NO_DEBUG_OUTPUT
    )
endif()

################################################################################

# Output?
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
    #ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib"
    #LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)

# Output?
install(TARGETS ${CMAKE_PROJECT_NAME}
    BUNDLE DESTINATION "${CMAKE_SOURCE_DIR}/bin"
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

################################################################################
