# 添加独立模块
function(add_module MODULE_NAME)
    set(options)

    set(oneValueArgs
        TYPE
    )

    set(multiValueArgs
        SOURCES
        PUBLIC_INCLUDES
        PRIVATE_INCLUDES
        PUBLIC_LIBS
        PRIVATE_LIBS
        PUBLIC_DEFINITIONS
        PRIVATE_DEFINITIONS
    )

    cmake_parse_arguments(
        MODULE
        "${options}"
        "${oneValueArgs}"
        "${multiValueArgs}"
        ${ARGN}
    )

    if(NOT MODULE_TYPE)
        set(MODULE_TYPE STATIC)
    endif()

    add_library(
        ${MODULE_NAME}
        ${MODULE_TYPE}
        ${MODULE_SOURCES}
    )

    target_include_directories(
        ${MODULE_NAME}
        PUBLIC
            ${MODULE_PUBLIC_INCLUDES}
        PRIVATE
            ${MODULE_PRIVATE_INCLUDES}
    )

    target_link_libraries(
        ${MODULE_NAME}
        PUBLIC
            ${MODULE_PUBLIC_LIBS}
        PRIVATE
            ${MODULE_PRIVATE_LIBS}
    )

    target_compile_definitions(
        ${MODULE_NAME}
        PUBLIC
            ${MODULE_PUBLIC_DEFINITIONS}
        PRIVATE
            ${MODULE_PRIVATE_DEFINITIONS}
    )

    target_compile_options(
        ${MODULE_NAME}
        PRIVATE
            -Wall
            -Wextra
            -Wpedantic
    )
endfunction()


# 添加纯头文件模块
function(add_header_module MODULE_NAME)
    set(options)
    set(oneValueArgs)

    set(multiValueArgs
        INCLUDES
        LIBS
        DEFINITIONS
    )

    cmake_parse_arguments(
        MODULE
        "${options}"
        "${oneValueArgs}"
        "${multiValueArgs}"
        ${ARGN}
    )

    add_library(
        ${MODULE_NAME}
        INTERFACE
    )

    if(MODULE_INCLUDES)
        target_include_directories(
            ${MODULE_NAME}
            INTERFACE
                ${MODULE_INCLUDES}
        )
    endif()

    if(MODULE_LIBS)
        target_link_libraries(
            ${MODULE_NAME}
            INTERFACE
                ${MODULE_LIBS}
        )
    endif()

    if(MODULE_DEFINITIONS)
        target_compile_definitions(
            ${MODULE_NAME}
            INTERFACE
                ${MODULE_DEFINITIONS}
        )
    endif()
endfunction()