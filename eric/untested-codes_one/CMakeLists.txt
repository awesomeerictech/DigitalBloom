# inside android/CMakeLists or main CMakeLists for Android build
set(NATIVE_SRCS
    AndroidServiceJNI.cpp
    # other sources...
)

add_library(digitalbloom SHARED ${NATIVE_SRCS} ...)

# Link to Qt libs if you use Qt types in JNI file:
find_package(Qt6 COMPONENTS Core REQUIRED)
target_include_directories(digitalbloom PRIVATE ${Qt6Core_INCLUDE_DIRS})
target_link_libraries(digitalbloom PRIVATE Qt6::Core Qt6::Network) # if used
# add other libraries as needed
