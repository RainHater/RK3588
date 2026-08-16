add_cpp_test(video_device_test
    SOURCE
        "${CMAKE_CURRENT_LIST_DIR}/video_device_test.cpp"

    LINK_LIBRARIES
        rkplatform_device_video
)
