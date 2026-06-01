include("C:/Qt/dev/qaterialdev/build/Desktop_Qt_6_9_3_MSVC2022_64bit-Release/cmake/CPM_0.40.2.cmake")
CPMAddPackage("NAME;QOlm;GIT_REPOSITORY;https://github.com/OlivierLDff/QOlm.git;GIT_TAG;v3.2.0;OPTIONS;QOLM_ENABLE_INSTALL OFF;QOLM_FOLDER_PREFIX Dependencies")
set(QOlm_FOUND TRUE)