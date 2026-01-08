set(BASE_LIB
    OpenCL
    pthread
)

set(OPENCV_LIB 
    opencv_imgcodecs
    opencv_highgui
    opencv_imgproc
    opencv_core
    opencv_videoio
    opencv_objdetect
    opencv_features2d
    opencv_flann
    opencv_calib3d  
    opencv_face
    opencv_freetype
    opencv_dnn
)

set(ROCKCHIP_LIB
    rknnrt
    rockchip_mpp
)

set_property(
    GLOBAL APPEND PROPERTY ALL_LIBS
    ${BASE_LIB}
    ${OPENCV_LIB}
    ${ROCKCHIP_LIB}
)

set_property(GLOBAL APPEND PROPERTY ALL_INCLUDE 
    ${CMAKE_INSTALL_PREFIX}/include
    ${CMAKE_INSTALL_PREFIX}/include/opencv4
)
