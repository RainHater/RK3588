add_module(rkplatform_device_inference
    TYPE STATIC

    SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/src/InferenceDevice.cpp

    PUBLIC_INCLUDES
        ${CMAKE_CURRENT_LIST_DIR}/include

    PRIVATE_LIBS
        rkplatform_bsp_rockchip_inference
)
