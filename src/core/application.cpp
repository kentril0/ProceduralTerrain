#include "pch.hpp"
#include "application.hpp"


// --------------------------------------------------------------------------
// Static variables
// --------------------------------------------------------------------------
//GLFWwindow* Application::window_obj;


// --------------------------------------------------------------------------
Application::Application(GLFWwindow* w, size_t initial_width, size_t initial_height) 
  : width(initial_width), 
    height(initial_height),
    lastFrame(glfwGetTime()), 
    framestamp(lastFrame)
{
    LOG_INFO("Screen Dimensions: " << width << " x " << height);

    // TODO "hide" the cursor for the camera
    glfwSetInputMode(w, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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
    // Setup OpenGL states
    // --------------------------------------------------------------------------
    glEnable(GL_DEPTH_TEST);

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
        LOG_INFO("Frametime: " << (1.0f / frames) << " ms");
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
    // Draw the scene
    // --------------------------------------------------------------------------
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, this->width, this->height);

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
    camera->on_mouse_move(x, y);
}

void Application::on_mouse_pressed(GLFWwindow *window, int button, int action, int mods)
{

}

void Application::on_key_pressed(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    camera->on_key_pressed(deltaTime, key, scancode, action, mods);
}

/**
 TODO
void Application::set_vsync(bool enabled)
{
    if (enabled)
        glfwSwapInterval(1);
    else
        glfwSwapInterval(0);
}
*/

