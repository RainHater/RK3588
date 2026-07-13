cmake_minimum_required(VERSION 3.16)

if(NOT DEFINED CMAKE_PROJECT_NAME)
    set(CMAKE_PROJECT_NAME app)
endif()

if(NOT DEFINED CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_LIST_DIR}/target")
endif()

project(${CMAKE_PROJECT_NAME} VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

set(CMAKE_SKIP_RPATH FALSE)
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_SKIP_INSTALL_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}")
set(CMAKE_PREFIX_PATH ${CMAKE_INSTALL_PREFIX})

set(SCRIPTS_PATH "${CMAKE_CURRENT_LIST_DIR}/scripts")
set(APP_LIB_PATH "${CMAKE_INSTALL_RPATH}/lib")

message("APP_LIB_PATH: ${APP_LIB_PATH}")

link_directories(${APP_LIB_PATH})
list(PREPEND CMAKE_PREFIX_PATH "${CMAKE_INSTALL_PREFIX}")

include(${SCRIPTS_PATH}/cmake/Dynamic.cmake)
include(${SCRIPTS_PATH}/cmake/Tools.cmake)

add_subdirectory(dynamics/utils)
add_subdirectory(dynamics/camera)
add_subdirectory(dynamics/platform/rockchip)

add_executable(${PROJECT_NAME} 
    ${CMAKE_CURRENT_LIST_DIR}/src/main.cpp
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    common_utils
    common_camera
    rockchip_inference
)

target_compile_options(${PROJECT_NAME} 
    PRIVATE 
    -Wall 
    -Wextra 
    -Wpedantic
)

INSTALL(TARGETS ${PROJECT_NAME} 
    DESTINATION bin
)

copy_directories_to_install_path(
    ${PROJECT_NAME}
    "${CMAKE_SOURCE_DIR}/config"
    "${CMAKE_SOURCE_DIR}/models"
    "${CMAKE_SOURCE_DIR}/tests"
)

include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests/facenet_test)
endif()
