add_library(rkplatform_bsp_video STATIC
    video/src/V4l2Capture.cpp
)

target_include_directories(rkplatform_bsp_video
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/video/include
        ${CMAKE_INSTALL_PREFIX}/include/opencv4
)

target_link_libraries(rkplatform_bsp_video
    PUBLIC
        rkplatform_component
    PRIVATE
        opencv_imgcodecs
        opencv_core
)

add_library(rkplatform_bsp_rockchip_media STATIC
    rockchip/media/src/FFmpegStreamer.cpp
)

target_include_directories(rkplatform_bsp_rockchip_media
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/rockchip/media/include
        ${CMAKE_INSTALL_PREFIX}/include/opencv4
        ${CMAKE_INSTALL_PREFIX}/include
)

target_link_libraries(rkplatform_bsp_rockchip_media
    PUBLIC
        rkplatform_component
    PRIVATE
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

add_library(rkplatform_bsp_rockchip_inference STATIC
    rockchip/inference/src/RknnInference.cpp
)

target_include_directories(rkplatform_bsp_rockchip_inference
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/rockchip/inference/include
        ${CMAKE_INSTALL_PREFIX}/include
)

target_link_libraries(rkplatform_bsp_rockchip_inference
    PUBLIC
        rkplatform_component
    PRIVATE
        rknnrt
)

foreach(target
    rkplatform_bsp_video
    rkplatform_bsp_rockchip_media
    rkplatform_bsp_rockchip_inference
)
    target_compile_options(${target}
        PRIVATE
            -Wall
            -Wextra
            -Wpedantic
    )
endforeach()
