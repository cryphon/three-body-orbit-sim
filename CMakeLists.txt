cmake_minimum_required(VERSION 3.10)
project(gravity_sim)

set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)

SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(${INCLUDE_DIR})

set(MAIN_SOURCE_FILES
    ${SRC_DIR}/main.cpp
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW3 REQUIRED glfw3)

# main executable
add_executable(${PROJECT_NAME} ${MAIN_SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE ${GLFW3_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${GLFW_LIBRARIES} glfw GLU GL)

# main
add_custom_target(run
    COMMAND ${PROJECT_NAME}
    DEPENDS ${PROJECT_NAME}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    COMMENT "Build and run gravity_sim"
)
