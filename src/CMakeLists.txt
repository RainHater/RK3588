add_subdirectory(camera)
add_subdirectory(core)
add_subdirectory(platform)
add_subdirectory(utils)
add_subdirectory(vision)


set_property(GLOBAL APPEND PROPERTY ALL_INCLUDE 
    ${CMAKE_CURRENT_LIST_DIR}
)

set_property(
    GLOBAL APPEND PROPERTY ALL_SOURCES 
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
)
