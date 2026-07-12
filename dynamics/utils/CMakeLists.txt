find_package(spdlog CONFIG REQUIRED)

add_dynamic_static_module(common_utils
    EXCLUDE_SOURCES
    INCLUDES
        ${CMAKE_CURRENT_LIST_DIR}/include
    LIBS
        pthread
        spdlog::spdlog
)
