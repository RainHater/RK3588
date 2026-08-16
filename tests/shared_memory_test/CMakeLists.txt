add_cpp_test(shared_memory_read
    NO_AUTO_RUN

    SOURCE
        "${CMAKE_CURRENT_LIST_DIR}/shared_memory_read.cpp"

    LINK_LIBRARIES
        rkplatform_platform

    COMMAND_ARGS
)

add_cpp_test(shared_memory_write
    NO_AUTO_RUN

    SOURCE
        "${CMAKE_CURRENT_LIST_DIR}/shared_memory_write.cpp"

    LINK_LIBRARIES
        rkplatform_platform

    COMMAND_ARGS
)

add_cpp_test(shared_memory_reopen
    SOURCE
        "${CMAKE_CURRENT_LIST_DIR}/shared_memory_reopen.cpp"

    LINK_LIBRARIES
        rkplatform_platform
)

add_test(
    NAME shared_memory_test
    COMMAND
        ${CMAKE_CURRENT_LIST_DIR}/run_shared_memory_test.sh
        $<TARGET_FILE:shared_memory_read>
        $<TARGET_FILE:shared_memory_write>
)
