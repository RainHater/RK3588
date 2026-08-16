add_cpp_test(facenet_test
    NO_AUTO_RUN

    SOURCE
        "${CMAKE_CURRENT_LIST_DIR}/facenet_test.cpp"

    LINK_LIBRARIES
            rkplatform_device_inference
            rkplatform_service_face
            rkplatform_component

            # OpenCV
            opencv_imgcodecs
            opencv_imgproc
            opencv_core

)

if(RKPLATFORM_ENABLE_HARDWARE_TESTS)
    add_test(
        NAME facenet_test
        COMMAND
            $<TARGET_FILE:facenet_test>
            "${CMAKE_SOURCE_DIR}/assets/models/facenet-float.rknn"
            "${CMAKE_SOURCE_DIR}/tests/datasets/1_001.jpg"
            "${CMAKE_SOURCE_DIR}/tests/datasets/1_002.jpg"
    )
endif()

add_cpp_test(face_processing_test
    SOURCE
        "${CMAKE_CURRENT_LIST_DIR}/face_processing_test.cpp"

    LINK_LIBRARIES
        rkplatform_service_face
        opencv_core
)
