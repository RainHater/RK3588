add_cpp_test(facenet_test
    SOURCE
        "${CMAKE_CURRENT_LIST_DIR}/facenet_test.cpp"

    LINK_LIBRARIES
            rockchip_inference
            common_utils

            # RKNN
            rknnrt

            # OpenCV
            opencv_imgcodecs
            opencv_imgproc
            opencv_core

    COMMAND_ARGS
        "${CMAKE_SOURCE_DIR}/assets/models/facenet-float.rknn"
        "${CMAKE_SOURCE_DIR}/tests/datasets/1_001.jpg"
        "${CMAKE_SOURCE_DIR}/tests/datasets/1_002.jpg"
)
