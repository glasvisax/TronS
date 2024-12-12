cmake_minimum_required(VERSION 3.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(PROJECT_NAME "TronS")
project(${PROJECT_NAME})

set(GLFW_INSTALL_DIR "D:/GLFW/install")
set(GLAD_SOURCE_DIR "D:/glad")
set(GLM_SOURCE_DIR "D:/glm/glm-1.0.1/glm")
set(ENET_INSTALL_DIR "D:/Enet/install")
set(IMGUI_INSTALL_DIR "D:/ImGui")

set(SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(BIN_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bin")

set(GLSL_EXT "glsl")

set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "${GLFW_INSTALL_DIR}/lib/cmake/glfw3")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${BIN_PATH}")

find_package(glfw3 CONFIG REQUIRED)

file(GLOB_RECURSE SRC_FILES 
    "${SRC_PATH}/*.cpp" 
    "${SRC_PATH}/*.h" 
    "${SRC_PATH}/*.${GLSL_EXT}"
)

file(GLOB_RECURSE GLAD_FILES 
    "${GLAD_SOURCE_DIR}/*.c" 
    "${GLAD_SOURCE_DIR}/*.h"
)

file(GLOB_RECURSE IMGUI_FILES 
    "${IMGUI_INSTALL_DIR}/*.cpp" 
    "${IMGUI_INSTALL_DIR}/*.h"
)

source_group(TREE "${SRC_PATH}" PREFIX "Source Files" FILES ${SRC_FILES})
source_group(TREE "${GLAD_SOURCE_DIR}" PREFIX "GLAD Files" FILES ${GLAD_FILES})
source_group(TREE "${IMGUI_INSTALL_DIR}" PREFIX "ImGui Files" FILES ${IMGUI_FILES})

set(SOURCES ${SRC_FILES} ${GLAD_FILES} ${IMGUI_FILES})

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE glfw PRIVATE "opengl32.lib" PRIVATE "${ENET_INSTALL_DIR}/lib/enet.lib" PRIVATE "ws2_32.lib" PRIVATE "winmm.lib")

target_include_directories(${PROJECT_NAME} PUBLIC "${GLAD_SOURCE_DIR}/include")
target_include_directories(${PROJECT_NAME} PUBLIC "${GLM_SOURCE_DIR}")
target_include_directories(${PROJECT_NAME} PUBLIC "${ENET_INSTALL_DIR}/include")
target_include_directories(${PROJECT_NAME} PUBLIC "${IMGUI_INSTALL_DIR}")

file(GLOB_RECURSE GLSL_FILES "${SRC_PATH}/*.${GLSL_EXT}")

add_custom_command(
    TARGET ${PROJECT_NAME}
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${GLSL_FILES} ${BIN_PATH}
)
