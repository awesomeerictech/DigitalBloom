
#set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON) # This is to find generated *.moc and *.h files in build dir

if(MSVC)
    add_compile_options("/Zc:__cplusplus")
    add_compile_options("/permissive-")
endif()

add_library(QSyncable OBJECT
    qsdiffrunner.cpp
    qspatch.cpp 
    qslistmodel.cpp
    qsuuid.cpp 
    qsdiffrunneralgo.cpp
    qstree.cpp
    qstreenode.cpp
    qsjsonlistmodel.cpp 
    qsyncableqmltypes.cpp 
    qsyncablefunctions.cpp
    qsyncableqmlwrapper.cpp
    )

set_target_properties(QSyncable PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED OFF
    CXX_EXTENSIONS OFF
)

target_compile_features(QSyncable PRIVATE cxx_std_17)


target_include_directories(QSyncable PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    $<TARGET_PROPERTY:Qt6::Core,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt6::Qml,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt6::Core5Compat,INTERFACE_INCLUDE_DIRECTORIES>
    )
