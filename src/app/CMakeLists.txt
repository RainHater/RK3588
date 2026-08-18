add_module(rkplatform_app_core
    TYPE STATIC

    SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/src/Application.cpp

    PUBLIC_INCLUDES
        ${CMAKE_CURRENT_LIST_DIR}/include

    PUBLIC_LIBS
        component_logging
)

add_executable(${PROJECT_NAME}
    ${CMAKE_CURRENT_LIST_DIR}/src/main.cpp
)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        APP_VERSION="${PROJECT_VERSION}"
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        rkplatform_app_core
        rkplatform_platform
)

target_compile_options(${PROJECT_NAME}
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)

install(TARGETS ${PROJECT_NAME}
    DESTINATION bin
)

copy_directories_to_install_path(
    ${PROJECT_NAME}
    "${CMAKE_SOURCE_DIR}/assets"
)
