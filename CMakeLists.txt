################################################################################
# Project: Procedural Terrain Generator
# Author: Martin Smutny, xsmutn13@stud.fit.vutbr.cz
# Date: 11th October 2020
#
# Project directory tree (approx., may not be up to date):
# ./
#  |-- build/
#  |-- images/
#  |-- src/
#  |   |-- libs/
#  |   |   |-- glad/
#  |   |   |-- glfw/
#  |   |   |-- single_header/
#  |   |-- core/
#  |   |-- opengl/
#  |   |-- scene/
#  |   |-- shaders/
#
################################################################################

cmake_minimum_required(VERSION 3.2)

# Project name
project(ProceduralTerrain)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(UNIX)
    find_package(OpenGL REQUIRED)
    find_package(X11 REQUIRED)
    find_package(Threads REQUIRED)
endif()

set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(SRC_CORE_DIR "${SRC_DIR}/core")


#-------------------------------------------------------------------------------
# EXTERNAL LIBRARIES:
#   GLFW
#   GLAD
#   Single Header (glm, stbi, ...)
#-------------------------------------------------------------------------------

include(ExternalProject)

set(LIBRARIES_DIR "${SRC_DIR}/libs")

#-------------------------------------------------------------------------------
# GLFW library
set(GLFW_INSTALL_DIR "${CMAKE_BINARY_DIR}/glfw")
set(GLFW_INCLUDE_DIR "${LIBRARIES_DIR}/glfw/include")
set(GLFW_LIBRARIES   "${GLFW_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}glfw3${CMAKE_STATIC_LIBRARY_SUFFIX}")

ExternalProject_Add(glfw
  SOURCE_DIR "${LIBRARIES_DIR}/glfw"

  DOWNLOAD_COMMAND ""
  UPDATE_COMMAND ""

  CMAKE_ARGS
      "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/glfw"
      "-DGLFW_BUILD_EXAMPLES=OFF"
      "-DGLFW_BUILD_TESTS=OFF"
      "-DGLFW_BUILD_DOCS=OFF"
      "-DBUILD_SHARED_LIBS=OFF"

  BUILD_BYPRODUCTS ${GLFW_LIBRARIES}
)

if(UNIX)
    set(GLFW_LIBRARIES ${GLFW_LIBRARIES} ${OPENGL_LIBRARIES} ${X11_LIBRARIES} 
        ${X11_Xrandr_LIB} ${X11_Xcursor_LIB} ${X11_Xinerama_LIB} ${X11_Xxf86vm_LIB}
        ${CMAKE_THREAD_LIBS_INIT}
    )
endif()


#-------------------------------------------------------------------------------
# GLAD library
set(GLAD_INCLUDE_DIR "${LIBRARIES_DIR}/glad/include")

add_library(glad "${LIBRARIES_DIR}/glad/src/glad.c")
target_include_directories(glad PRIVATE "${GLAD_INCLUDE_DIR}")

set(GLAD_LIBARIES)
if(UNIX)
    set(GLAD_LIBRARIES "glad" ${CMAKE_DL_LIBS})
else()
    set(GLAD_LIBRARIES glad)
endif()

#--------------------------------------------------------------------------------
# Single header libraries
set(SINGLE_HEADER_LIBS_INCLUDE_DIR "${LIBRARIES_DIR}/single_header")

# Configure compiler flags for our project
if (CMAKE_CXX_COMPILER_ID MATCHES Clang OR ${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
    # using regular Clang or AppleClang or GCC
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall")
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    # using Visual Studio C++
    set(CMAKE_CXX_FLAGS "/std:c++17 /permissive- /W3 /EHsc")
endif()


#--------------------------------------------------------------------------------
# Project
#--------------------------------------------------------------------------------
set(SRC_CORE_DIR "${SRC_DIR}/core")
set(SRC_SHADERS_DIR "${SRC_DIR}/shaders")
set(IMAGES_DIR "${CMAKE_SOURCE_DIR}/images")

#--------------------------------------------------------------------------------
# opengl interface
set(SRC_OPENGL_DIR "${SRC_DIR}/opengl")
add_library(opengl
    "${SRC_OPENGL_DIR}/buffer.cpp"
    "${SRC_OPENGL_DIR}/buffer.hpp"
    "${SRC_OPENGL_DIR}/shader.cpp"
    "${SRC_OPENGL_DIR}/shader.hpp"
    "${SRC_OPENGL_DIR}/texture2d.cpp"
    "${SRC_OPENGL_DIR}/texture2d.hpp"
    "${SRC_OPENGL_DIR}/vertex_array.cpp"
    "${SRC_OPENGL_DIR}/vertex_array.hpp"
)
add_dependencies(opengl glad)
target_link_libraries(opengl
    ${GLAD_LIBRARIES}
)
target_include_directories(opengl
    PRIVATE ${GLAD_INCLUDE_DIR}
    PRIVATE ${SINGLE_HEADER_LIBS_INCLUDE_DIR}
    PRIVATE ${SRC_DIR}
)

#--------------------------------------------------------------------------------
# scene objects 
set(SRC_SCENE_DIR "${SRC_DIR}/scene")
add_library(scene
    "${SRC_SCENE_DIR}/camera.hpp"
    "${SRC_SCENE_DIR}/camera.cpp"
    "${SRC_SCENE_DIR}/skybox.hpp"
    "${SRC_SCENE_DIR}/skybox.cpp"
    "${SRC_SCENE_DIR}/terrain.hpp"
    "${SRC_SCENE_DIR}/terrain.cpp"
)
add_dependencies(scene glad opengl)
target_link_libraries(scene
    ${GLAD_LIBRARIES}
    opengl
)
target_include_directories(scene
    PRIVATE ${GLAD_INCLUDE_DIR}
    PRIVATE ${SINGLE_HEADER_LIBS_INCLUDE_DIR}
    PRIVATE ${SRC_DIR}
    PRIVATE ${SRC_SHADERS_DIR}
    PRIVATE ${IMAGES_DIR}
    #PRIVATE ${SRC_OPENGL_DIR}
)


add_executable(${CMAKE_PROJECT_NAME}
    "${SRC_CORE_DIR}/main.cpp"
    "${SRC_CORE_DIR}/application.cpp"
    "${SRC_CORE_DIR}/utilities.cpp"
)

add_dependencies(${CMAKE_PROJECT_NAME}
    glfw
    glad
    opengl
    scene
)

target_link_libraries(${CMAKE_PROJECT_NAME}
    ${GLFW_LIBRARIES}
    ${GLAD_LIBARIES}
    opengl
    scene
)

target_include_directories(${CMAKE_PROJECT_NAME}
    PRIVATE ${GLFW_INCLUDE_DIR}
    PRIVATE ${GLAD_INCLUDE_DIR}
    PRIVATE ${SINGLE_HEADER_LIBS_INCLUDE_DIR}
    PRIVATE ${SRC_DIR}
    PRIVATE ${CMAKE_SOURCE_DIR}
)

