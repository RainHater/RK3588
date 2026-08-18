add_module(rkplatform_device_video
    TYPE STATIC

    SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/src/VideoDevice.cpp

    PUBLIC_INCLUDES
        ${CMAKE_CURRENT_LIST_DIR}/include

    PRIVATE_LIBS
        rkplatform_bsp_video
        rkplatform_bsp_rockchip_media
)
