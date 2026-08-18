# 增加测试程序
function(add_cpp_test TARGET_NAME)
    set(options
        NO_AUTO_RUN
    )

    set(oneValueArgs
        SOURCE
    )

    set(multiValueArgs
        LINK_LIBRARIES
        COMMAND_ARGS
    )

    cmake_parse_arguments(
        ARG
        "${options}"
        "${oneValueArgs}"
        "${multiValueArgs}"
        ${ARGN}
    )

    if(NOT ARG_SOURCE)
        message(
            FATAL_ERROR
            "add_cpp_test(${TARGET_NAME}): SOURCE is required"
        )
    endif()

    add_executable(
        ${TARGET_NAME}
        "${ARG_SOURCE}"
    )

    if(ARG_LINK_LIBRARIES)
        target_link_libraries(
            ${TARGET_NAME}
            PRIVATE
                ${ARG_LINK_LIBRARIES}
        )
    endif()

    target_compile_options(
        ${TARGET_NAME}
        PRIVATE
            -Wall
            -Wextra
            -Wpedantic
    )

    if(NOT ARG_NO_AUTO_RUN)
        add_test(
            NAME ${TARGET_NAME}
            COMMAND
                $<TARGET_FILE:${TARGET_NAME}>
                ${ARG_COMMAND_ARGS}
        )
    endif()
endfunction()