WT_ADD_EXAMPLE(qrlogin.wt 
  QRAuth.C
  QRAuthWidget.C
  model/QRAuthService.C
  model/QRTokenDatabase.C
  model/Session.C
  model/User.C
)

TARGET_LINK_LIBRARIES(qrlogin.wt ${EXAMPLES_DBO_LIBS})

if(WIN32)
    target_link_libraries(qrlogin.wt liblzma libzstd_static zlibstatic )
else()
    target_link_libraries(qrlogin.wt  liblzma libzstd_static z )
endif()

INCLUDE_DIRECTORIES(${WT_SOURCE_DIR}/src)
