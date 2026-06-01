# src/CMakeLists.txt
#
# Copyright (C) 2008  Werner Smekal
#
# create library
# =======================================================================
# libhpdf source files
# =======================================================================
set(
    LIBHPDF_SRCS
    hpdf_annotation.c
    hpdf_array.c
    hpdf_binary.c
    hpdf_boolean.c
    hpdf_catalog.c
    hpdf_destination.c
    hpdf_dict.c
    hpdf_direct.c
    hpdf_doc_png.c
    hpdf_doc.c
    hpdf_encoder_cns.c
    hpdf_encoder_cnt.c
    hpdf_encoder_jp.c
    hpdf_encoder_kr.c
    hpdf_encoder_utf.c
    hpdf_encoder.c
    hpdf_encrypt.c
    hpdf_encryptdict.c
    hpdf_error.c
    hpdf_ext_gstate.c
    hpdf_font_cid.c
    hpdf_font_tt.c
    hpdf_font_type1.c
    hpdf_font.c
    hpdf_fontdef_base14.c
    hpdf_fontdef_cid.c
    hpdf_fontdef_cns.c
    hpdf_fontdef_cnt.c
    hpdf_fontdef_jp.c
    hpdf_fontdef_kr.c
    hpdf_fontdef_tt.c
    hpdf_fontdef_type1.c
    hpdf_fontdef.c
    hpdf_gstate.c
    hpdf_image_ccitt.c
    hpdf_image_png.c
    hpdf_image.c
    hpdf_info.c
    hpdf_list.c
    hpdf_mmgr.c
    hpdf_name.c
    hpdf_namedict.c
    hpdf_null.c
    hpdf_number.c
    hpdf_objects.c
    hpdf_outline.c
    hpdf_page_label.c
    hpdf_page_operator.c
    hpdf_pages.c
    hpdf_real.c
    hpdf_shading.c
    hpdf_streams.c
    hpdf_string.c
    hpdf_u3d.c
    hpdf_utils.c
    hpdf_xref.c
    hpdf_pdfa.c
    hpdf_3dmeasure.c
    hpdf_exdata.c
    hpdf_encoder_utf.c
)

set_property(SOURCE hpdf_shading.c hpdf_font_tt.c hpdf_fontdef_tt.c
  PROPERTY
    SKIP_UNITY_BUILD_INCLUSION ON)

# =======================================================================
# create hpdf library
# =======================================================================
add_library(hpdf ${LIBHPDF_SRCS})
set_target_properties(hpdf PROPERTIES
    SOVERSION ${HPDF_MAJOR_VERSION}.${HPDF_MINOR_VERSION}
    VERSION ${HPDF_MAJOR_VERSION}.${HPDF_MINOR_VERSION}.${HPDF_BUGFIX_VERSION}
)

if(WIN32)
    target_include_directories(hpdf PRIVATE
        ${CMAKE_SOURCE_DIR}/wt/boost/libarchive/zlib
        ${CMAKE_BINARY_DIR}/wt/boost/libarchive/zlib
        ${CMAKE_SOURCE_DIR}/wt/libharu/include
        ${CMAKE_BINARY_DIR}/wt/libharu/include
    )
else()
    target_include_directories(hpdf PRIVATE
	    ${CMAKE_SOURCE_DIR}/wt/boost/libarchive/zlib
        ${CMAKE_BINARY_DIR}/wt/boost/libarchive/zlib
        ${CMAKE_SOURCE_DIR}/wt/libharu/include
        ${CMAKE_BINARY_DIR}/wt/libharu/include
    )
endif()




target_compile_options(hpdf PRIVATE
    $<$<C_COMPILER_ID:MSVC>:/DZ_PREFIX=0>
    $<$<NOT:$<C_COMPILER_ID:MSVC>>:-DZ_PREFIX=0>
)
# Also explicitly define it to 0 to be very explicit
target_compile_definitions(hpdf PRIVATE Z_PREFIX=0)

#target_compile_definitions(hpdf PRIVATE Z_PREFIX)

if (PNG_FOUND)
    include_directories (${PNG_INCLUDE_DIRS})
    target_link_libraries (hpdf ${PNG_LIBRARIES})
endif()
if (ZLIB_FOUND)
    include_directories(${ZLIB_INCLUDE_DIRS})

    if (UNIX)
        target_link_libraries(hpdf ${ZLIB_LIBRARIES})
    else()
        #target_link_libraries(hpdf ${ZLIB_LIBRARIES} zlib)
		target_link_libraries(hpdf zlibstatic)
    endif()
endif()

if(WIN32)
    target_link_libraries (hpdf zlibstatic)
else()
    target_link_libraries (hpdf zlibstatic)
endif()



# Math library
if(UNIX AND NOT APPLE AND NOT EMSCRIPTEN)
    target_link_libraries (hpdf ${M_LIB})
endif()

install(
   TARGETS hpdf
   ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
   LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
   RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
