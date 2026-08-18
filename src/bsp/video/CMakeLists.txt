add_module(rkplatform_bsp_video
    TYPE STATIC
    
    SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/src/V4l2Capture.cpp

    PUBLIC_INCLUDES
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${CMAKE_INSTALL_PREFIX}/include/opencv4

    PUBLIC_LIBS
        component_logging

    PRIVATE_LIBS
        opencv_imgcodecs
        opencv_core
)