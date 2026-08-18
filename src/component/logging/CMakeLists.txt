find_package(spdlog CONFIG REQUIRED)

add_module(component_logging
    TYPE STATIC
    
    SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/src/Logger.cpp

    PUBLIC_INCLUDES
        ${CMAKE_CURRENT_LIST_DIR}/include

    PUBLIC_LIBS
        spdlog::spdlog
)