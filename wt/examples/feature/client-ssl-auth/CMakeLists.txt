IF(HAVE_SSL)

WT_ADD_EXAMPLE(client-ssl-auth.wt 
  SslClientAuth.C
  model/Session.C
  model/User.C
)

TARGET_LINK_LIBRARIES(client-ssl-auth.wt ${EXAMPLES_DBO_LIBS})
if(WIN32)
    target_link_libraries(client-ssl-auth.wt liblzma libzstd_static zlibstatic )
else()
    target_link_libraries(client-ssl-auth.wt  liblzma libzstd_static z )
endif()

INCLUDE_DIRECTORIES(${WT_SOURCE_DIR}/src)

ENDIF(HAVE_SSL)
