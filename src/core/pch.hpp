#pragma once

#include <iostream>
#include <memory>

#include <string>
#include <vector>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>

#include <glad/glad.h>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext.hpp>

#include "utilities.hpp"

#define PROJECT_NAME "Procedural Terrain Generator"

// TODO turn on debug outputs
#define DEBUG

// TODO enable asserts
#define ENABLE_ASSERTS

// TODO define logging level
// Logging messages which are less severe than LOG_LEVEL will be ignored
#define LOG_LEVEL LEVEL_OK

#include "log.hpp"

// TODO OpenGL version
#define OPENGL_VERSION_MAJOR 4
#define OPENGL_VERSION_MINOR 5
#define GLSL_VERSION_STR "#version 450"

// TODO initial screen dimensions
#define SCREEN_INIT_WIDTH  1280
#define SCREEN_INIT_HEIGHT 720 


