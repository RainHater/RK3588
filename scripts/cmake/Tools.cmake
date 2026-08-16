function(copy_directory_to_target TARGET_NAME SOURCE_DIR DEST_DIR_NAME)
    if(NOT TARGET ${TARGET_NAME})
        message(FATAL_ERROR "Target '${TARGET_NAME}' does not exist")
    endif()

    if(NOT IS_DIRECTORY "${SOURCE_DIR}")
        message(FATAL_ERROR "Directory '${SOURCE_DIR}' does not exist")
    endif()

    add_custom_command(
        TARGET ${TARGET_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${SOURCE_DIR}"
            "$<TARGET_FILE_DIR:${TARGET_NAME}>/${DEST_DIR_NAME}"
        COMMENT
            "Copying ${SOURCE_DIR} to ${DEST_DIR_NAME}"
        VERBATIM
    )
endfunction()

# 复制单个文件夹
function(copy_directory_to_install_path TARGET_NAME SOURCE_DIR)
    if(NOT TARGET ${TARGET_NAME})
        message(FATAL_ERROR
            "Target '${TARGET_NAME}' does not exist")
    endif()

    if(NOT IS_DIRECTORY "${SOURCE_DIR}")
        message(FATAL_ERROR
            "Directory '${SOURCE_DIR}' does not exist")
    endif()

    get_filename_component(
        SOURCE_DIR_ABS
        "${SOURCE_DIR}"
        ABSOLUTE
    )

    get_filename_component(
        SOURCE_DIR_NAME
        "${SOURCE_DIR_ABS}"
        NAME
    )

    set(
        INSTALL_DESTINATION
        "${CMAKE_INSTALL_PREFIX}/bin/${SOURCE_DIR_NAME}"
    )

    add_custom_command(
        TARGET ${TARGET_NAME}
        POST_BUILD

        COMMAND ${CMAKE_COMMAND} -E make_directory
            "${INSTALL_DESTINATION}"

        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${SOURCE_DIR_ABS}"
            "${INSTALL_DESTINATION}"

        COMMENT
            "Copying ${SOURCE_DIR_NAME} to ${INSTALL_DESTINATION}"

        VERBATIM
    )
endfunction()


# 批量复制多个文件夹
function(copy_directories_to_install_path TARGET_NAME)
    foreach(SOURCE_DIR IN LISTS ARGN)
        copy_directory_to_install_path(
            ${TARGET_NAME}
            "${SOURCE_DIR}"
        )
    endforeach()
endfunction()

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

    # 没有指定 NO_AUTO_RUN 才注册到 CTest
    if(NOT ARG_NO_AUTO_RUN)
        add_test(
            NAME ${TARGET_NAME}
            COMMAND
                $<TARGET_FILE:${TARGET_NAME}>
                ${ARG_COMMAND_ARGS}
        )
    endif()
endfunction()