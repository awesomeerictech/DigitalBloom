cmake_minimum_required(VERSION 3.20)

project(MobileSharing LANGUAGES CXX)


set(QT_NO_PRIVATE_MODULE_WARNING ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

################################################################################

# Generic dependencies
find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml)
set(CORE_LIBRARIES Qt::Core Qt::Gui Qt::Qml)

# Generic sources
set(CORE_SOURCES
    SharingApplication.cpp SharingApplication.h
    SharingUtils.cpp SharingUtils.h)

# OS specific sources & dependencies
if (ANDROID)
    find_package(Qt6 REQUIRED COMPONENTS CorePrivate)
    set(PLATFORM_LIBRARIES Qt::CorePrivate)
    set(PLATFORM_SOURCES
        SharingUtils_android.cpp
        SharingUtils_android.h)
elseif (IOS)
    find_package(Qt6 REQUIRED COMPONENTS Quick)
    set(PLATFORM_LIBRARIES Qt::Quick)
    set(PLATFORM_SOURCES
        SharingUtils_ios.mm
        docviewcontroller_ios.mm)
endif()

################################################################################

add_library(MobileSharing OBJECT ${CORE_SOURCES} ${PLATFORM_SOURCES})
add_library(MobileSharing::MobileSharing ALIAS MobileSharing)

set_target_properties(MobileSharing PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED OFF
    CXX_EXTENSIONS OFF
)

#target_compile_features(MobileSharing PRIVATE cxx_std_17)


target_link_libraries(MobileSharing PRIVATE ${CORE_LIBRARIES} ${PLATFORM_LIBRARIES})
target_include_directories(MobileSharing PUBLIC ${CMAKE_CURRENT_LIST_DIR})
