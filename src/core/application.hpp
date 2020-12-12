#pragma once

#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "scene/skybox.hpp"
#include "scene/camera.hpp"
#include "scene/terrain.hpp"


/**@brief Controls used in application */
enum Controls
{
    KEY_TOGGLE_MENU  = GLFW_KEY_ESCAPE,
    KEY_CAM_FORWARD  = GLFW_KEY_W,
    KEY_CAM_BACKWARD = GLFW_KEY_S,
    KEY_CAM_RIGHT    = GLFW_KEY_D,
    KEY_CAM_LEFT     = GLFW_KEY_A,
    KEY_CAM_RCURSOR  = KEY_TOGGLE_MENU
};


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

    void call_registered(int key, int action)
    { 
        const auto& it = callback_map.find({key,action,state});
        if (it != callback_map.end())
        {
            const auto& vec_callbacks = it->second;
            for (const auto& c : vec_callbacks)
                (this->*c)();
        }
      //(this->*(callback_map[{key,action,state}]))(); 
    }

    // Functions modifying application state
    void set_state(int s) { /* DERR("New state: " << s); */ state = s; }

    void set_state_modify();    ///< Shows interface for modifications
    void set_state_freefly();   ///< Hides interface and enables flying w/ camera

    // ImGui functions
    void show_interface();

    void status_window();

    void showTexture(uint32_t texture_id, const glm::uvec2& texSize,
                     const float w, const float h);

    // Camera callbacks
    void camera_key_pressed() { camera->on_key_pressed(key, key_action); }
    void camera_forward()     { camera->key_forward(key_action); }
    void camera_backward()    { camera->key_backward(key_action); }
    void camera_right()       { camera->key_right(key_action); }
    void camera_left()        { camera->key_left(key_action); }
    void camera_reset()       { camera->key_reset(key_action); }

private:
    // ----------------------------------------------------------------------------
    // Typedefs
    // ----------------------------------------------------------------------------
    // TODO callback class??
    typedef void (Application::*Callback)(void);
    typedef std::vector<Callback> callbacks;

    /** @brief Used as a key to a map of callbacks, that are called whenever a key
     *         with action and during a state is pressed. */
    struct CallbackKey
    {
        int key;            ///< The GLFW_KEY_* - a.k.a. key on a keyboard
        int action;         ///< Usually a GLFW_PRESS or GLFW_RELEASE
        int state;          ///< The application state

        CallbackKey(int k, int a, int s) : key(k), action(a), state(s) {}
        bool operator==(const CallbackKey &other) const
        {
            return (key == other.key
                    && action == other.action
                    && state == other.state);
        }
    };

    // Make the CallbackKey type hashable 
    struct Callback_hash
    {
        std::size_t operator()(const CallbackKey& k) const
        {
            // Compute individual hash values and combine them 
            //  using XOR and bit shifting:
            return ((std::hash<int>()(k.key) 
                     ^ (std::hash<int>()(k.action) << 1)) >> 1) 
                     ^ (std::hash<int>()(k.state) << 1);
        }
    };

    // ----------------------------------------------------------------------------
    // Data members 
    // ----------------------------------------------------------------------------
    GLFWwindow* window;

    size_t width;
    size_t height;

    // Timestamps
    double lastFrame, framestamp, deltaTime;
    uint32_t frames;

    // Maps the key, action and state to callback function
    std::unordered_map<CallbackKey, callbacks, Callback_hash> callback_map;

    int key, key_action;        ///< Keyboard key and action

    enum States     ///< Application states, determines 
    {
        STATE_MODIFY,   ///< GUI is shown and allows for modifications
        STATE_FREEFLY   ///< GUI is hidden, camera is moving freely
    };

    int state;

    // ----------------------------------------------------------------------------
    // Scene
    // TODO scene objects, scene shaders, etc.
    // TODO resource manager?
    // TODO active camera, walk camera
    // ----------------------------------------------------------------------------
    glm::mat4 projView;

    std::unique_ptr<Camera> camera;
    void cameraSetPresetTop();
    void cameraSetPresetFront();
    void cameraSetPresetSideWays();

    std::unique_ptr<Skybox> skybox;

    std::unique_ptr<Terrain> terrain;
    const uint32_t TERRAIN_INIT_SIZE = 100;
    bool m_wireframe = false;

    

};

