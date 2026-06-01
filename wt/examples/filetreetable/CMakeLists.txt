IF(NOT (WT_CPP17_FILESYSTEM_IMPLEMENTATION STREQUAL "std" OR BOOST_FS_LIB))
  MESSAGE(STATUS "** Not building filetreetable example: requires std::filesystem or boost_filesystem library.")
ELSE()

  WT_ADD_EXAMPLE(filetreetable.wt
    FileTreeTable.C
    FileTreeTableNode.C
    FileTreeExample.C
  )

  TARGET_LINK_LIBRARIES(filetreetable.wt
    ${BOOST_FS_LIB}
    ${BOOST_SYSTEM_LIB}
  )

if(WIN32)
    target_link_libraries(filetreetable.wt liblzma libzstd_static zlibstatic )
else()
    target_link_libraries(filetreetable.wt  liblzma libzstd_static z )
endif()

  INCLUDE_DIRECTORIES(${WT_SOURCE_DIR}/src)

ENDIF()

