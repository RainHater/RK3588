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
        "${CMAKE_INSTALL_PREFIX}/bin/models/facenet-float.rknn"
        "${CMAKE_INSTALL_PREFIX}/bin/tests/datasets/1_001.jpg"
        "${CMAKE_INSTALL_PREFIX}/bin/tests/datasets/1_002.jpg"
)
