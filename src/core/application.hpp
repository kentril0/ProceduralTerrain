#pragma once

#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "scene/skybox.hpp"
#include "scene/camera.hpp"
#include "scene/terrain.hpp"


/**
 * @brief Main application that gets rendered in the setup window
 */
class Application
{
public:
    Application(GLFWwindow* w, size_t initial_width, size_t initial_height);

    ~Application();

    void loop();

    // ----------------------------------------------------------------------------
    // Input events
    // ----------------------------------------------------------------------------
    void on_resize(GLFWwindow *window, int width, int height);
    void on_mouse_move(GLFWwindow *window, double x, double y);
    void on_mouse_pressed(GLFWwindow *window, int button, int action, 
                          int mods);
    void on_key_pressed(GLFWwindow *window, int key, int scancode, int action, 
                        int mods);

private:
    void render();

    void update();

    void set_vsync(bool enabled);

private:
    //static GLFWwindow* window_obj;
    size_t width;
    size_t height;

    // Timestamps
    double lastFrame, framestamp, deltaTime;
    uint32_t frames;

    // ----------------------------------------------------------------------------
    // Scene
    // TODO scene objects, scene shaders, etc.
    // TODO resource manager?
    // TODO active camera, walk camera
    // ----------------------------------------------------------------------------
    std::unique_ptr<Camera> camera;

    std::shared_ptr<Shader> sh_skybox;
    std::unique_ptr<Skybox> skybox;

    std::unique_ptr<Shader> sh_terrain;
    std::unique_ptr<Terrain> terrain;
};

