add_subdirectory(ipc)
add_subdirectory(process)

# 平台层聚合 Target，供上层（app/tests）统一引用
add_library(rkplatform_platform INTERFACE)
target_link_libraries(rkplatform_platform
    INTERFACE
        platform_ipc
        platform_process
)
