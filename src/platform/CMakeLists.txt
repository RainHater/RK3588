add_library(rkplatform_platform INTERFACE)

target_include_directories(rkplatform_platform
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/ipc/include
        ${CMAKE_CURRENT_LIST_DIR}/process/include
)

target_link_libraries(rkplatform_platform
    INTERFACE
        rkplatform_component
        pthread
)
