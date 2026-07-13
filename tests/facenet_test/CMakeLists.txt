add_executable(facenet_test
    ${CMAKE_CURRENT_LIST_DIR}/facenet_test.cpp
)

target_link_libraries(facenet_test
    PRIVATE
        rockchip_inference
        common_utils

        # RKNN
        rknnrt

        # OpenCV
        opencv_imgcodecs
        opencv_imgproc
        opencv_core
)

target_compile_options(facenet_test
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)

add_test(
    NAME facenet_inference_test

    COMMAND facenet_test
        "${CMAKE_INSTALL_PREFIX}/bin/models/facenet-float.rknn"
        "${CMAKE_INSTALL_PREFIX}/bin/tests/datasets/1_001.jpg"
        "${CMAKE_INSTALL_PREFIX}/bin/tests/datasets/1_002.jpg"
)