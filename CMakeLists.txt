cmake_minimum_required(VERSION 3.16)

if(NOT DEFINED CMAKE_PROJECT_NAME)
    set(CMAKE_PROJECT_NAME app)
endif()

if(NOT DEFINED APP_VERSION)
    set(APP_VERSION "1.0.0")
endif()

if(NOT DEFINED CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_LIST_DIR}/target")
endif()

project(${CMAKE_PROJECT_NAME} VERSION ${APP_VERSION} LANGUAGES CXX)

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

include(${SCRIPTS_PATH}/cmake/Tools.cmake)

add_subdirectory(src)

include(CTest)
option(RKPLATFORM_ENABLE_HARDWARE_TESTS "Enable tests that require target hardware" OFF)
if(BUILD_TESTING)
    add_subdirectory(tests/component_test)
    add_subdirectory(tests/device_test)
    add_subdirectory(tests/facenet_test)
    add_subdirectory(tests/shared_memory_test)
endif()
