add_module(rkplatform_bsp_rockchip_media
    TYPE STATIC
    
    SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/src/FFmpegStreamer.cpp

    PUBLIC_INCLUDES
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${CMAKE_INSTALL_PREFIX}/include/opencv4
        ${CMAKE_INSTALL_PREFIX}/include

    PUBLIC_LIBS
        component_logging

    PRIVATE_LIBS
        avformat
        avcodec
        avutil
        swresample
        swscale
        rockchip_mpp
        rga
        opencv_imgproc
        opencv_core
)