function(add_dynamic_static_module MODULE_NAME)
    # EXCLUDE_SOURCES       排除的文件
    # INCLUDES              头文件路径
    # LIBS                  链接的库

    cmake_parse_arguments(MODULE "" "" "SOURCES;EXCLUDE_SOURCES;INCLUDES;LIBS" ${ARGN})

    if(NOT MODULE_SOURCES)
        file(GLOB_RECURSE MODULE_SOURCES
            CONFIGURE_DEPENDS
            ${CMAKE_CURRENT_LIST_DIR}/*.cpp
            ${CMAKE_CURRENT_LIST_DIR}/*.c
        )
    endif()

    if(MODULE_EXCLUDE_SOURCES)
        foreach(EXCLUDE_SOURCE ${MODULE_EXCLUDE_SOURCES})
            if(IS_ABSOLUTE ${EXCLUDE_SOURCE})
                list(REMOVE_ITEM MODULE_SOURCES ${EXCLUDE_SOURCE})
            else()
                list(REMOVE_ITEM MODULE_SOURCES ${CMAKE_CURRENT_LIST_DIR}/${EXCLUDE_SOURCE})
            endif()
        endforeach()
    endif()

    add_library(${MODULE_NAME} STATIC
        ${MODULE_SOURCES}
    )

    target_include_directories(${MODULE_NAME}
        PUBLIC
        ${MODULE_INCLUDES}
    )

    target_link_libraries(${MODULE_NAME}
        PUBLIC
        ${MODULE_LIBS}
    )

    add_custom_command(TARGET ${MODULE_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_INSTALL_PREFIX}/lib
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:${MODULE_NAME}>
            ${CMAKE_INSTALL_PREFIX}/lib/
    )
endfunction()
