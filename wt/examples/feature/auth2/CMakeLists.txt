WT_ADD_EXAMPLE(auth2.wt 
  Auth2.C
  AuthWidget.C
  RegistrationView.C
  model/Session.C
  model/User.C
  model/UserDetailsModel.C
)

TARGET_LINK_LIBRARIES(auth2.wt ${EXAMPLES_DBO_LIBS})

if(WIN32)
    target_link_libraries(auth2.wt liblzma libzstd_static zlibstatic )
else()
    target_link_libraries(auth2.wt  liblzma libzstd_static z )
endif()


INCLUDE_DIRECTORIES(${WT_SOURCE_DIR}/src)
