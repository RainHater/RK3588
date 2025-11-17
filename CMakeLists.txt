cmake_minimum_required(VERSION 3.16)

if(DEFINED PROJECT_NAME)
    set(CMAKE_PROJECT_NAME ${PROJECT_NAME})
else()
    set(CMAKE_PROJECT_NAME CppTemplate)
endif()

project(${CMAKE_PROJECT_NAME} VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

set_property(GLOBAL APPEND PROPERTY ALL_SOURCES "")
set_property(GLOBAL APPEND PROPERTY ALL_INCLUDE "")
set_property(GLOBAL APPEND PROPERTY ALL_DEFINE "")

add_subdirectory(src)

get_property(ALL_SOURCES GLOBAL PROPERTY ALL_SOURCES)
get_property(ALL_INCLUDE GLOBAL PROPERTY ALL_INCLUDE)
get_property(ALL_DEFINE  GLOBAL PROPERTY ALL_DEFINE)

message("宏定义: ${ALL_DEFINE}")
message("头文件: ${ALL_INCLUDE}")
message("源文件: ${ALL_SOURCES}")

add_executable(${PROJECT_NAME} ${ALL_SOURCES})

include_directories(${ALL_INCLUDE})

if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()
