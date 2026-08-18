add_cpp_test(thread_pool_test
    SOURCE
        "${CMAKE_CURRENT_LIST_DIR}/thread_pool_test.cpp"

    LINK_LIBRARIES
        component_concurrency
)

add_cpp_test(logger_test
    SOURCE
        "${CMAKE_CURRENT_LIST_DIR}/logger_test.cpp"

    LINK_LIBRARIES
        component_logging
)
