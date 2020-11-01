#include "pch.hpp"
#include "application.hpp"


// --------------------------------------------------------------------------
// Static variables
// --------------------------------------------------------------------------


// --------------------------------------------------------------------------
Application::Application(GLFWwindow* w, size_t initial_width, size_t initial_height) 
  : window(w),
    width(initial_width), 
    height(initial_height),
    state(STATE_MODIFY)
{
    LOG_INFO("Screen Dimensions: " << width << " x " << height);

    // "Show" the cursor
    glfwSetInputMode(w, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // --------------------------------------------------------------------------
    // TODO initialize TODO
    // --------------------------------------------------------------------------
    camera = std::make_unique<Camera>(float(width) / float(height));
    
    sh_skybox = std::make_shared<Shader>("shaders/draw_skybox.vs", 
                                         "shaders/draw_skybox.fs");

    skybox = std::make_unique<Skybox>(sh_skybox, 
                                      std::initializer_list<const char*>{
                                       "images/skybox/right.jpg",
                                       "images/skybox/left.jpg",
                                       "images/skybox/top.jpg",
                                       "images/skybox/bottom.jpg",
                                       "images/skybox/front.jpg",
                                       "images/skybox/back.jpg"});

    sh_terrain = std::make_unique<Shader>("shaders/draw_terrain.vs",
                                          "shaders/draw_terrain.fs");
    terrain = std::make_unique<Terrain>(32, 32,
      std::make_shared<Texture2D>("images/farmland.jpg", false),
      std::make_shared<Texture2D>("images/Grass0130_seamless.jpg", false),
      std::make_shared<Texture2D>("images/Snow0163_seamless.jpg", false));

    // --------------------------------------------------------------------------
    // Register callbacks
    // --------------------------------------------------------------------------
    callback_map = {
        // Key,             Action,     State
        { {KEY_TOGGLE_MENU, GLFW_PRESS, STATE_MODIFY}, {&Application::set_state_freefly} },
        { {KEY_TOGGLE_MENU, GLFW_PRESS, STATE_FREEFLY}, {&Application::set_state_modify, 
                                                         &Application::camera_reset} },
        // Camera
        { {KEY_CAM_FORWARD, GLFW_PRESS, STATE_FREEFLY}, {&Application::camera_forward} },
        { {KEY_CAM_FORWARD, GLFW_RELEASE, STATE_FREEFLY}, {&Application::camera_forward} },
        { {KEY_CAM_BACKWARD,GLFW_PRESS, STATE_FREEFLY}, {&Application::camera_backward} },
        { {KEY_CAM_BACKWARD,GLFW_RELEASE, STATE_FREEFLY}, {&Application::camera_backward} },
        { {KEY_CAM_RIGHT,   GLFW_PRESS, STATE_FREEFLY}, {&Application::camera_right} },
        { {KEY_CAM_RIGHT,   GLFW_RELEASE, STATE_FREEFLY}, {&Application::camera_right} },
        { {KEY_CAM_LEFT,    GLFW_PRESS, STATE_FREEFLY}, {&Application::camera_left} },
        { {KEY_CAM_LEFT,    GLFW_RELEASE, STATE_FREEFLY}, {&Application::camera_left} }
    };

    // --------------------------------------------------------------------------
    // Setup OpenGL states
    // --------------------------------------------------------------------------
    glEnable(GL_DEPTH_TEST);

    set_vsync(false);


    // --------------------------------------------------------------------------
    // Get current timestamp - prepare for main loop
    // --------------------------------------------------------------------------
    lastFrame = glfwGetTime();
    framestamp = lastFrame;
}

Application::~Application()
{

}

void Application::loop()
{
    // --------------------------------------------------------------------------
    // Calculate delta time
    // --------------------------------------------------------------------------
    double currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // Frametime and FPS counter, updates once per 1 second
    if (currentFrame - framestamp > 1.0f)
    {
        // TODO
        // Frametime
        LOG_INFO("Frametime: " << (1000.0f / frames) << " ms");
        //fmtcntr->set_text("Ftime: " + std::to_string(1.0f / frames) + " ms");

        // FPS
        LOG_INFO("FPS: " << (frames));
        //fpscntr->set_text("FPS: " + std::to_string(frames));
        
        framestamp += 1.0f;
        frames = 0;
    }

    update();

    render();
}

void Application::render()
{
    // --------------------------------------------------------------------------
    // Clear and reset
    // --------------------------------------------------------------------------
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, this->width, this->height);

    // --------------------------------------------------------------------------
    // Sart the Dear ImGUI frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    // --------------------------------------------------------------------------
    // Draw the scene
    // --------------------------------------------------------------------------
    // Get projection and view matrices defined by camera
    glm::mat4 proj = camera->get_proj_matrix();
    glm::mat4 view = camera->get_view_matrix();

    // Render the terrain
    sh_terrain->use();
    sh_terrain->set_mat4("model", terrain->get_model());
    sh_terrain->set_mat4("view", view);
    sh_terrain->set_mat4("proj", proj);
    terrain->render();

    // Render skybox as last
    skybox->render(view, proj);

    
    // --------------------------------------------------------------------------
    // ImGUI render
    // --------------------------------------------------------------------------
    
    // By default GUI is shown
    {
        show_interface();
    }
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    // --------------------------------------------------------------------------

    frames++;
}

void Application::update()
{
    camera->update(deltaTime);
}


void Application::on_resize(GLFWwindow *window, int width, int height)
{
    this->width = width;
    this->height = height;
    LOG_INFO("SCREEN RESIZE");
}

void Application::on_mouse_move(GLFWwindow *window, double x, double y) 
{ 
    if (state == STATE_FREEFLY)
        camera->on_mouse_move(x, y);
}

void Application::on_mouse_pressed(GLFWwindow *window, int button, int action, int mods)
{

}

void Application::on_key_pressed(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    this->key = key;
    key_action = action;

    call_registered(key, action);
}

void Application::show_interface()
{
    if (state == STATE_MODIFY)
    {
        // TODO fix on rescale
        //ImGui::ShowDemoWindow(NULL);
        if (!ImGui::Begin("Application controls", NULL))
        {
            ImGui::End();
            return;
        }

        static bool vsync = false;
        if (ImGui::Checkbox(" Vertical sync", &vsync))
            set_vsync(vsync);
        
        // application settings
        //   vsync : checkox
        //   antialiasing : combobox 2, 4, 8
        // camera settings
        //  position vec3   ; input float3
        //  speed slider    : input float
        //  zoom            :  input float
        //  fov             :   input float
        //  near            : input float
        //  far             : input float
        // terrain controls
        //  dimensions      : input int (?)
        //  noise functions 
        // sky effects
        //  light postion   : input float3
        //  light color     : color1
        //  fog color       : color1
        //  fog falloff     : input float
        ImGui::End();
    }

    status_window();
}

void Application::status_window()
{
    // Overlay when flying with camera
    if (state == STATE_FREEFLY)
        ImGui::SetNextWindowBgAlpha(0.35f);

    // TODO fix on rescale
    // Collapsed or Clipped
    if (!ImGui::Begin("Application Metrics", NULL))
    {
        ImGui::End();
        return;
    }

        ImGuiIO& io = ImGui::GetIO();
        // frametime and FPS
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 
                    1000.0f / io.Framerate, io.Framerate);
        ImGui::Text("%d vertices, %d indices (%d triangles)", 
                    io.MetricsRenderVertices, io.MetricsRenderIndices, 
                    io.MetricsRenderIndices / 3);
        ImGui::Text("%d active allocations", io.MetricsActiveAllocations);

        // TODO terrain score
        // current camera position
    
    ImGui::End();
}

void Application::set_state_modify()
{
    set_state(STATE_MODIFY);

    // Show the cursor
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void Application::set_state_freefly()
{
    set_state(STATE_FREEFLY);

    // Hide the cursor
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Application::set_vsync(bool enabled)
{
    if (enabled)
        glfwSwapInterval(1);
    else
        glfwSwapInterval(0);
}

