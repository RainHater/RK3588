add_dynamic_static_module(common_camera
    EXCLUDE_SOURCES
    INCLUDES
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${CMAKE_INSTALL_PREFIX}/include/opencv4
        ${CMAKE_INSTALL_PREFIX}/include
    LIBS
        common_utils
        # FFMPEG
        avformat
        avcodec
        avutil
        swresample
        swscale
        # Rockchip
        rknnrt
        rockchip_mpp
        rga
        # Opencv
        opencv_imgcodecs
        opencv_highgui
        opencv_imgproc
        opencv_core
        opencv_videoio 
)
