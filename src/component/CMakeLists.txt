find_package(spdlog CONFIG REQUIRED)

add_library(rkplatform_component STATIC
    logging/src/Logger.cpp
)

target_include_directories(rkplatform_component
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/logging/include
        ${CMAKE_CURRENT_LIST_DIR}/concurrency/include
)

target_link_libraries(rkplatform_component
    PUBLIC
        spdlog::spdlog
)

target_compile_options(rkplatform_component
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)
