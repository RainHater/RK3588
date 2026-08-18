add_module(rkplatform_service_face
    TYPE STATIC

    SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/src/FaceProcessing.cpp

    PUBLIC_INCLUDES
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${CMAKE_INSTALL_PREFIX}/include/opencv4

    PRIVATE_LIBS
        opencv_imgproc
        opencv_core
)
