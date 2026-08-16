add_library(rkplatform_device_inference STATIC
    inference/src/InferenceDevice.cpp
)

target_include_directories(rkplatform_device_inference
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/inference/include
)

target_link_libraries(rkplatform_device_inference
    PRIVATE
        rkplatform_bsp_rockchip_inference
)

target_compile_options(rkplatform_device_inference
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)

add_library(rkplatform_device_video STATIC
    video/src/VideoDevice.cpp
)

target_include_directories(rkplatform_device_video
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/video/include
)

target_link_libraries(rkplatform_device_video
    PRIVATE
        rkplatform_bsp_video
        rkplatform_bsp_rockchip_media
)

target_compile_options(rkplatform_device_video
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)
