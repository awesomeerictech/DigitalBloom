
#set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON) # This is to find generated *.moc and *.h files in build dir

set(COMMON_SOURCES
        src/statusbar.cpp
        src/statusbar.h
    )
	
if(MSVC)
    add_compile_options("/Zc:__cplusplus")
    add_compile_options("/permissive-")
endif()


set(PLATFORM_SOURCES "")

# iOS-specific source files
if (CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
    message(STATUS "Configuring for iOS")
    list(APPEND PLATFORM_SOURCES
        src/statusbar_ios.mm
    )
# Windows-specific source files
elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    message(STATUS "Configuring for Windows")
    list(APPEND PLATFORM_SOURCES
        src/statusbar_dummy.cpp
    )
# Android-specific source files
elseif (ANDROID)
    message(STATUS "Configuring for Android")
    list(APPEND PLATFORM_SOURCES
        src/statusbar_android.cpp
    )

endif()

# ==============================================================================
# Define the library 
# ==============================================================================
add_library(StatusBar OBJECT
    ${COMMON_SOURCES}
    ${PLATFORM_SOURCES}
	
    )

set_target_properties(StatusBar PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED OFF
    CXX_EXTENSIONS OFF
)

target_compile_features(StatusBar PRIVATE cxx_std_17)


# ==============================================================================
# Link platform-specific libraries
# ==============================================================================
if (CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
    # For iOS, link the UIKit framework.
    # The `find_library` command is more robust than a direct string.
    find_library(UIKIT_FRAMEWORK UIKit)
    if(UIKIT_FRAMEWORK)
        target_link_libraries(StatusBar PRIVATE ${UIKIT_FRAMEWORK})
    else()
        message(FATAL_ERROR "UIKit framework not found for iOS.")
    endif()
endif()


target_include_directories(StatusBar PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    $<TARGET_PROPERTY:Qt6::Core,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt6::Core5Compat,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt6::Qml,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt6::Quick,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt6::Network,INTERFACE_INCLUDE_DIRECTORIES>
    )
