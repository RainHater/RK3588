find_package(OpenMP REQUIRED)

if(OpenMP_CXX_FOUND OR OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    add_definitions(-fopenmp)
    message(STATUS "find OpenMP")
endif()

find_package(OpenCL REQUIRED)
if (OpenCL_FOUND)
    include_directories(${OpenCL_INCLUDE_DIRS})
    message(STATUS "find OpenCL: ${OpenCL_LIBRARIES}")
endif()

set(BASE_LIB
    OpenCL
    pthread
)

set(SPDLOG_LIB
    
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
    turbojpeg
)

set(FFMPEG_LIB
    avformat
    avcodec
    avutil
    swresample
    swscale
)

set(ROCKCHIP_LIB
    rknnrt
    rockchip_mpp
    rga
)

set_property(
    GLOBAL APPEND PROPERTY ALL_LIBS
    ${BASE_LIB}
    ${OPENCV_LIB}
    ${ROCKCHIP_LIB}
    ${SPDLOG_LIB}
    ${FFMPEG_LIB}
)

set_property(GLOBAL APPEND PROPERTY ALL_INCLUDE 
    ${CMAKE_INSTALL_PREFIX}/include
    ${CMAKE_INSTALL_PREFIX}/include/opencv4
    ${CMAKE_INSTALL_PREFIX}/include/rknpu2
)
