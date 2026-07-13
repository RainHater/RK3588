add_dynamic_static_module(platform_inference
    EXCLUDE_SOURCES
    INCLUDES
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${CMAKE_INSTALL_PREFIX}/include/opencv4
        ${CMAKE_INSTALL_PREFIX}/include
    LIBS
        common_utils
        # Opencv
        opencv_core
)
