add_module(rkplatform_bsp_rockchip_inference
    TYPE STATIC

    SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/src/RknnInference.cpp

    PUBLIC_INCLUDES
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${CMAKE_INSTALL_PREFIX}/include

    PUBLIC_LIBS
        component_logging

    PRIVATE_LIBS
        rknnrt
)