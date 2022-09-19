# Copyright (c) 2022 ProceduralTerrain authors Distributed under MIT License 
#(http://opensource.org/licenses/MIT)

cmake_minimum_required(VERSION 3.16)

project(terrain)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


#-------------------------------------------------------------------------------
# Set Directories

set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(SHADERS_DIR "${CMAKE_SOURCE_DIR}/shaders")
set(TEXTURES_DIR "${CMAKE_SOURCE_DIR}/textures")
set(LIBS_DIR "${CMAKE_SOURCE_DIR}/libs")


#-------------------------------------------------------------------------------
# Libraries

set(SGL_DIR "${LIBS_DIR}/SGL")
set(SGL_INCLUDE_DIR "${LIBS_DIR}/SGL/src")

Set(SGL_BUILD_EXAMPLES OFF)

add_subdirectory(${SGL_DIR})

#--------------------------------------------------------------------------------
# Project
#--------------------------------------------------------------------------------
set(SRC_SCENE_DIR "${SRC_DIR}/scene")

set(sources 
    "${SRC_DIR}/main.cpp"
    "${SRC_DIR}/ResourceManager.cpp"
    "${SRC_SCENE_DIR}/Terrain.cpp"
    "${SRC_SCENE_DIR}/Skybox.cpp"
    "${SRC_SCENE_DIR}/Camera.cpp"
    "${SRC_SCENE_DIR}/ProceduralTexture2D.cpp"
    "${SRC_DIR}/GUI.cpp"
    "${SRC_DIR}/ProceduralTerrain.cpp"
)

#--------------------------------------------------------------------------------
add_executable(${CMAKE_PROJECT_NAME} ${sources})

add_dependencies(${CMAKE_PROJECT_NAME}
    SGL
)

#set (CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -ldl")

target_link_libraries(${CMAKE_PROJECT_NAME}
    SGL
)

target_include_directories(${CMAKE_PROJECT_NAME}
    PRIVATE "${SGL_DIR}" ${SRC_DIR}
)

#--------------------------------------------------------------------------------
# Copy assets to build folder
#--------------------------------------------------------------------------------

add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${SHADERS_DIR}
    ${PROJECT_BINARY_DIR}/shaders
    COMMENT "Copy shaders to build tree")

add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${TEXTURES_DIR}
    ${PROJECT_BINARY_DIR}/textures
    COMMENT "Copy textures to build tree")

