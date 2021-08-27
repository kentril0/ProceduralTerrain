# OpenGL Procedural Terrain Generation
Procedural Terrain Generator using OpenGL and C++

<img src="images/version_1_0.png" width="640" height="360">

<br/>Without shading:<br/>
<img src="images/non_shaded.png" width="640" height="360">

Shading using Phong Model:<br/>
<img src="images/shaded.png" width="640" height="360">

## Version
1.0

## Features
* Interactive GUI for tweaking your terrain
* FPS camera
* Skybox
* Procedural Mesh
* Accumulated Perlin Noise
* Texturing
* Falloff map
* Height-based Blending
* Shading using Phong model

## How to compile

### Requirements:
* C++17
* min. CMake 3.16
* min. OpenGL 4.5 compatible GPU

In root directory of this project:
* mkdir build && cd build
* cmake ..
* make
* ./proc_ter_gen

## Used libraries
* GLAD
* GLFW3
* GLM
* STB_Image
* Tiny Obj Loader
* ImGUI

## Known Issues
TODO
