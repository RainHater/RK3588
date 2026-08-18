add_header_module(platform_ipc
    INCLUDES
        ${CMAKE_CURRENT_LIST_DIR}/include

    LIBS
        component_logging
        pthread
)
