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
    // TODO local
    terrain = std::make_unique<Terrain>(TERRAIN_INIT_SIZE, TERRAIN_INIT_SIZE);
      // TODO
      //std::make_shared<Texture2D>("images/farmland.jpg", false),
      //std::make_shared<Texture2D>("images/Grass0130_seamless.jpg", false),
      //std::make_shared<Texture2D>("images/Snow0163_seamless.jpg", false));

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

    set_vsync(true);


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
        //LOG_INFO("Frametime: " << (1000.0f / frames) << " ms");
        //fmtcntr->set_text("Ftime: " + std::to_string(1000.0f / frames) + " ms");

        // FPS
        //LOG_INFO("FPS: " << (frames));
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
    sh_terrain->set_mat4("model", terrain->model());
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

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
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

static void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void Application::show_interface()
{
    if (state == STATE_MODIFY)
    {
        // TODO fix on rescale
        ImGui::ShowDemoWindow(NULL);


        if (!ImGui::Begin("Application Controls", NULL))
        {
            ImGui::End();
            return;
        }

        if (ImGui::CollapsingHeader("Configuration"))
        {
            static bool vsync = true;
            if (ImGui::Checkbox(" Vertical sync", &vsync))
                set_vsync(vsync);
            // TODO UI scaling
        }

        if (ImGui::CollapsingHeader("Camera Settings"))
        {
            // TODO help (?)
            glm::vec3 pos = camera->get_position();
            float pitch = camera->get_pitch();
            float yaw = camera->get_yaw();
            static float fov = glm::radians(camera->get_field_of_view());
            static float near = camera->get_near_plane_dist();
            static float far = camera->get_far_plane_dist();

            if (ImGui::InputFloat3("Position", glm::value_ptr(pos)))
                camera->set_position(pos);

            if (ImGui::SliderFloat("Pitch angle", &pitch, -89.f, 89.f, "%.0f deg"))
                camera->set_pitch(pitch);

            if (ImGui::SliderFloat("Yaw angle", &yaw, 0.f, 360.f, "%.0f deg"))
                camera->set_yaw(yaw);

            if (ImGui::SliderAngle("Field of view", &fov, 0.f, 180.f))
                camera->set_field_of_view(fov);

            if (ImGui::SliderFloat("Near plane", &near, 0.f, 10.f))
                camera->set_near_plane_dist(near);

            if (ImGui::SliderFloat("Far plane", &far, 100.f, 1000.f))
                camera->set_far_plane_dist(far);

            // TODO camera preset positions relative to terrain size
            
        }
        if (ImGui::CollapsingHeader("Terrain Controls", ImGuiTreeNodeFlags_DefaultOpen))
        {
            {
                static int dim = TERRAIN_INIT_SIZE;
                float tileScale = terrain->tileScale();
                float heightScale = terrain->heightScale();

                // Dimensions
                // TODO ? with  [vertices^2]
                if (ImGui::SliderInt("Terrain size", &dim, 4, 512))
                    terrain->setSize(dim, dim);
                if (ImGui::SliderFloat("Tile scale", &tileScale, 0.01f, 10.f))
                    terrain->setTileScale(tileScale);
                if (ImGui::SliderFloat("Height scale", &heightScale, 0.01f, 10.f))
                    terrain->setHeightScale(heightScale);
            }
            ImGui::Separator();
            if (ImGui::TreeNodeEx("Noise Map Generation", ImGuiTreeNodeFlags_DefaultOpen))
            {
                static const ProceduralTex2D& noiseMap = terrain->heightMap();
                int32_t seed = noiseMap.seed();
                if (ImGui::DragInt("Seed", &seed))
                    noiseMap.reseed(seed);
                    
                // Select noise function
                static int noiseType = noiseMap.type();
                ImGui::RadioButton("Random Noise", &noiseType, Noise::Type::Random); 
                    //ImGui::SameLine();
                ImGui::RadioButton("Perlin Noise", &noiseType, Noise::Type::Perlin2D);
                ImGui::RadioButton("Accumulated Perlin Noise", &noiseType, 
                                   Noise::Type::OctavesPerlin2D);

                if (noiseType == Noise::Type::OctavesPerlin2D)
                {
                    int32_t octaves = noiseMap.octaves();
                    float persistence = noiseMap.persistence();
                    // TODO (?)
                    if (ImGui::SliderInt("Octaves", &octaves, 1, 10))
                        noiseMap.setOctaves(octaves);
                    if (ImGui::SliderFloat("Persistence", &persistence, 0.f, 1.f))
                        noiseMap.setPersistence(persistence);
                }

                noiseMap.setType(static_cast<Noise::Type>(noiseType));

                // TODO function
                // Visualize the procedural texture
                {
                    static ImTextureID my_tex_id = (void*)(intptr_t)(procTex.ID());
                    glm::uvec2 tex_size = procTex.size();

                    const float my_tex_w = 128;
                    const float my_tex_h = 128;

                    ImGui::BeginGroup();
                        ImGui::Text("%u x %u", tex_size.x, tex_size.y);
                        ImVec2 pos = ImGui::GetCursorScreenPos();
                        ImVec2 uv_min = ImVec2(0.0f, 0.0f);                 // Top-left
                        ImVec2 uv_max = ImVec2(1.0f, 1.0f);                 // Lower-right
                        ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
                        ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white
                        ImGui::Image(my_tex_id, ImVec2(my_tex_w, my_tex_h), uv_min, 
                                     uv_max, tint_col, border_col);
                        if (ImGui::IsItemHovered())
                        {
                            ImGuiIO& io = ImGui::GetIO();
                            ImGui::BeginTooltip();
                            float region_sz = 32.0f;
                            float region_x = io.MousePos.x - pos.x - region_sz * 0.5f;
                            float region_y = io.MousePos.y - pos.y - region_sz * 0.5f;
                            float zoom = 4.0f;
                            if (region_x < 0.0f) { region_x = 0.0f; }
                            else if (region_x > my_tex_w - region_sz) { 
                                region_x = my_tex_w - region_sz; }
                            if (region_y < 0.0f) { region_y = 0.0f; }
                            else if (region_y > my_tex_h - region_sz) {
                                region_y = my_tex_h - region_sz; }
                            ImGui::Text("Min: (%.2f, %.2f)", region_x, region_y);
                            ImGui::Text("Max: (%.2f, %.2f)", region_x + region_sz, 
                                                             region_y + region_sz);
                            ImVec2 uv0 = ImVec2((region_x) / my_tex_w, (region_y) / my_tex_h);
                            ImVec2 uv1 = ImVec2((region_x + region_sz) / my_tex_w, 
                                                (region_y + region_sz) / my_tex_h);
                            ImGui::Image(my_tex_id, ImVec2(region_sz * zoom, 
                                                           region_sz * zoom), 
                                         uv0, uv1, tint_col, border_col);
                            ImGui::EndTooltip();
                        }
                    ImGui::EndGroup();
                }
                if (ImGui::Button("Apply"))
                {
                    terrain->applyNoiseMap();
                }
                ImGui::TreePop();
            }
            ImGui::Separator();
            // TODO procedural texturing
            if (ImGui::TreeNode("Texturing"))
            {
                static int use_type = 0;
                const uint8_t TERR_COLORS = 0;
                const uint8_t TERR_TEXTURES = 1;
                ImGui::RadioButton("Use colors", &use_type, TERR_COLORS); ImGui::SameLine();
                ImGui::RadioButton("Use textures", &use_type, TERR_TEXTURES);

                // Terrain Colors
                if (use_type == TERR_COLORS)
                {
                    static glm::vec3 color1(1.0f);
                    static glm::vec3 color2(1.0f);
                    static glm::vec3 color3(1.0f);
                    ImGui::ColorEdit3("Color 1", glm::value_ptr(color1)); ImGui::SameLine(); 
                    HelpMarker("Click on the colored square to open a color picker.\n"
                               "Click and hold to use drag and drop.\n"
                               "Right-click on the colored square to show options.\n"
                               "CTRL+click on individual component to input value.\n");
                    ImGui::ColorEdit3("Color 2", glm::value_ptr(color2));
                    ImGui::ColorEdit3("Color 3", glm::value_ptr(color3));
                }
                // Textures
                else if (use_type == TERR_TEXTURES)
                {
                    // TODO read https://github.com/ocornut/imgui/wiki/Image-Loading-and-Displaying-Examples
                    // TODO scale icons
                    ImGuiIO& io = ImGui::GetIO();
                    ImGui::TextWrapped("Hello, below are images of textures");
                    //ImTextureID my_tex_id = io.Fonts->TexID;
                    //float my_tex_w = 96;
                    //float my_tex_h = 96;
                    
                    //ImTextureID my_tex_id = &((void)(terrain->heightMap().ID()));
                    uint32_t tex_id = terrain->heightMap().ID();
                    glm::uvec2 tex_size = terrain->heightMap().size();

                    //glBindTexture(GL_TEXTURE_2D, my_tex_id);
                    ImTextureID my_tex_id = (void*)(intptr_t)tex_id;
                    
                    float my_tex_w = 96;
                    float my_tex_h = 96;

                    for (int i = 0; i < 3; ++i)
                    {
                      if (i > 0)
                        ImGui::SameLine();

                      ImGui::BeginGroup();
                        ImGui::Text("%u x %u", tex_size.x, tex_size.y);
                        ImVec2 pos = ImGui::GetCursorScreenPos();
                        ImVec2 uv_min = ImVec2(0.0f, 0.0f);                 // Top-left
                        ImVec2 uv_max = ImVec2(1.0f, 1.0f);                 // Lower-right
                        ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
                        ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white
                        ImGui::Image(my_tex_id, ImVec2(my_tex_w, my_tex_h), uv_min, 
                                     uv_max, tint_col, border_col);
                        if (ImGui::IsItemHovered())
                        {
                            ImGui::BeginTooltip();
                            float region_sz = 32.0f;
                            float region_x = io.MousePos.x - pos.x - region_sz * 0.5f;
                            float region_y = io.MousePos.y - pos.y - region_sz * 0.5f;
                            float zoom = 4.0f;
                            if (region_x < 0.0f) { region_x = 0.0f; }
                            else if (region_x > my_tex_w - region_sz) { 
                                region_x = my_tex_w - region_sz; }
                            if (region_y < 0.0f) { region_y = 0.0f; }
                            else if (region_y > my_tex_h - region_sz) {
                                region_y = my_tex_h - region_sz; }
                            ImGui::Text("Min: (%.2f, %.2f)", region_x, region_y);
                            ImGui::Text("Max: (%.2f, %.2f)", region_x + region_sz, 
                                                             region_y + region_sz);
                            ImVec2 uv0 = ImVec2((region_x) / my_tex_w, (region_y) / my_tex_h);
                            ImVec2 uv1 = ImVec2((region_x + region_sz) / my_tex_w, 
                                                (region_y + region_sz) / my_tex_h);
                            ImGui::Image(my_tex_id, ImVec2(region_sz * zoom, 
                                                           region_sz * zoom), 
                                         uv0, uv1, tint_col, border_col);
                            ImGui::EndTooltip();
                        }
                      ImGui::EndGroup();
                    }
                }
            // Slope controls
                ImGui::TreePop();
            }
            

            
            

            // TODO noise functions
            //  Each texture has a range, 2 - 512
            //      if last res != res: update
            //
        }

        if (ImGui::CollapsingHeader("Sky effects"))
        {
            static glm::vec3 light_col(1.0f);
            static glm::vec3 fog_col(1.0f);
            static float fog_falloff = 1.0f;

            ImGui::ColorEdit3("Light color", glm::value_ptr(light_col));
            ImGui::ColorEdit3("Fog color", glm::value_ptr(fog_col));
            ImGui::SliderFloat("Fog falloff dist", &fog_falloff, 1.f, 1000.f);
        }

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
                    terrain->totalVertices(), terrain->totalIndices(),
                    terrain->totalIndices() / 3);

        // TODO terrain score
        // current camera position
    
    ImGui::End();
}

void Application::set_state_modify()
{
    set_state(STATE_MODIFY);

    // Show the cursor
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    // TODO
    // ImGui::SetNextWindowSize(ImVec2(width * 0.3, height * 0.8));
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

