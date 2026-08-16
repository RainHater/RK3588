add_library(rkplatform_app_core STATIC
    Application.cpp
)

target_include_directories(rkplatform_app_core
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(rkplatform_app_core
    PUBLIC
        rkplatform_component
)

target_compile_options(rkplatform_app_core
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)

add_executable(${PROJECT_NAME}
    main.cpp
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
