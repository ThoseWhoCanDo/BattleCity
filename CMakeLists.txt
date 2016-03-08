project(BattleCity)
cmake_minimum_required(VERSION 2.8)
add_subdirectory(${PROJECT_SOURCE_DIR}/src)

include_directories(${HEADER_DIRS})

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

add_subdirectory(${PROJECT_SOURCE_DIR}/deps/glfw)
include_directories(${PROJECT_SOURCE_DIR}/deps/glfw/include)
target_link_libraries(${PROJECT_NAME} glfw ${GLFW_LIBRARIES})

add_subdirectory(${PROJECT_SOURCE_DIR}/deps/glm)
include_directories(${PROJECT_SOURCE_DIR}/deps/glm)

set(CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/deps/glew/out")
find_package(GLEW REQUIRED)
if (GLEW_FOUND)
    include_directories(${GLEW_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${GLEW_LIBRARIES})
endif()
