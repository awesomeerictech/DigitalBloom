cmake_minimum_required(VERSION 3.1.3)


#list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

project(StreamServer LANGUAGES C CXX)
#set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON) # This is to find generated *.moc and *.h files in build dir
set(_MY_LINK_PUBLIC_STREAM)  # empty initially, append conditionally where PUBLIC is required
set(_MY_LINK_PRIVATE_STREAM)
set(_MY_INCLUDES_PUBLIC_STREAM)
set(_MY_INCLUDES_PRIVATE_STREAM)

add_subdirectory(${CMAKE_SOURCE_DIR}/mystreamserver/ZLMediaKit)

if(USE_ZL)
    list(APPEND _MY_INCLUDES_PUBLIC_STREAM
        ${CMAKE_SOURCE_DIR}/mystreamserver/ZLMediaKit/3rdpart
        ${CMAKE_SOURCE_DIR}/mystreamserver/ZLMediaKit/3rdpart/media-server/libflv/include
        ${CMAKE_SOURCE_DIR}/mystreamserver/ZLMediaKit/3rdpart/media-server/libmov/include
        ${CMAKE_SOURCE_DIR}/mystreamserver/ZLMediaKit/3rdpart/media-server/libmpeg/include
        ${CMAKE_SOURCE_DIR}/mystreamserver/ZLMediaKit/3rdpart/ZLToolKit/src
        ${CMAKE_SOURCE_DIR}/mystreamserver/ZLMediaKit/src
        ${CMAKE_SOURCE_DIR}/mystreamserver/ZLMediaKit/ext-codec
        ${CMAKE_SOURCE_DIR}/mystreamserver/ZLMediaKit/srt
    )

    if(ANDROID)
        list(APPEND _MY_LINK_PRIVATE_STREAM
            srt flv mov mpeg zltoolkit ext-codec zlmediakit log
            ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY}
        )
    else()
        list(APPEND _MY_LINK_PRIVATE_STREAM
            srt flv mov mpeg zltoolkit ext-codec zlmediakit
            ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY}
        )
    endif()

    # Optional alternative:
    # list(APPEND _MY_LINK_PRIVATE_STREAM srt webrtc flv mov mpeg zltoolkit ext-codec zlmediakit mk_api)

endif()

if(MSVC)
   # add_compile_options("/Zc:__cplusplus")
   # add_compile_options("/permissive-")
endif()

add_library(StreamServer OBJECT
    zlm_wrapper_api.h  zlm_wrapper.cpp
    )
	
set_target_properties(StreamServer PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

target_compile_features(StreamServer PRIVATE cxx_std_11)


target_compile_definitions(StreamServer PRIVATE
     ENABLE_HLS
     ENABLE_MP4
     ENABLE_OPENSSL
     ENABLE_RTPPROXY
     ENABLE_VERSION
     ENABLE_X264
     mk_api_EXPORTS
)

if (_MY_INCLUDES_PUBLIC_STREAM)
    target_include_directories(StreamServer PUBLIC
    ${_MY_INCLUDES_PUBLIC_STREAM}
    ${CMAKE_CURRENT_LIST_DIR}
    #$<TARGET_PROPERTY:Qt6::Core,INTERFACE_INCLUDE_DIRECTORIES>
    #$<TARGET_PROPERTY:Qt6::Qml,INTERFACE_INCLUDE_DIRECTORIES>
    #$<TARGET_PROPERTY:Qt6::Core5Compat,INTERFACE_INCLUDE_DIRECTORIES>
    )
endif()
if (_MY_INCLUDES_PRIVATE_STREAM)
    target_include_directories(StreamServer PRIVATE ${_MY_INCLUDES_PRIVATE_STREAM})
endif()


# One call to target_link_libraries with aggregated PUBLIC and PRIVATE lists
# (Ordering of IMPORTED libs follows the append order above)
if (_MY_LINK_PUBLIC_STREAM)
    if (_MY_LINK_PRIVATE_STREAM)
        target_link_libraries(StreamServer PRIVATE ${_MY_LINK_PRIVATE_STREAM} PUBLIC ${_MY_LINK_PUBLIC_STREAM})
    else()
        target_link_libraries(StreamServer PUBLIC ${_MY_LINK_PUBLIC_STREAM})
    endif()
else()
    if (_MY_LINK_PRIVATE_STREAM)
        target_link_libraries(StreamServer PRIVATE ${_MY_LINK_PRIVATE_STREAM})
    endif()
endif()

# Done.
