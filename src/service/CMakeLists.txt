add_library(rkplatform_service_face STATIC
    face/src/FaceProcessing.cpp
)

target_include_directories(rkplatform_service_face
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/face/include
        ${CMAKE_INSTALL_PREFIX}/include/opencv4
)

target_link_libraries(rkplatform_service_face
    PRIVATE
        opencv_imgproc
        opencv_core
)

target_compile_options(rkplatform_service_face
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)
