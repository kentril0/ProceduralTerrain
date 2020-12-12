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
    cameraSetPresetSideWays();
    
    std::shared_ptr<Shader> sh_skybox = std::make_shared<Shader>(
                                         "shaders/draw_skybox.vs", 
                                         "shaders/draw_skybox.fs");
    skybox = std::make_unique<Skybox>(sh_skybox, 
                                      std::initializer_list<const char*>{
                                       "images/skybox/right.jpg",
                                       "images/skybox/left.jpg",
                                       "images/skybox/top.jpg",
                                       "images/skybox/bottom.jpg",
                                       "images/skybox/front.jpg",
                                       "images/skybox/back.jpg"});

    std::shared_ptr<Shader> sh_terrain = std::make_shared<Shader>(
                                          "shaders/draw_terrain.vs",
                                          "shaders/draw_terrain.fs");
    terrain = std::make_unique<Terrain>(sh_terrain,
                                        TERRAIN_INIT_SIZE, TERRAIN_INIT_SIZE, 
                                        0.2f, 2.f);
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
    projView = proj * view;

    // Render the terrain
    if (m_wireframe)
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

    terrain->render(projView);

    if (m_wireframe)
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

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
    DERR("SCREEN RESIZE");
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
    ImGui::SameLine();
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
        //ImGui::ShowDemoWindow(NULL);

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
            if (ImGui::Checkbox(" Wireframe", &m_wireframe)) {}


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
            ImGui::Text("Position Presets");
            ImGui::Separator();
            if (ImGui::Button("From Top"))
                cameraSetPresetTop();
            ImGui::SameLine();
            if (ImGui::Button("From Front"))
                cameraSetPresetFront();
            ImGui::SameLine();
            if (ImGui::Button("From Side"))
                cameraSetPresetSideWays();

            ImGui::NewLine();
        }
        if (ImGui::CollapsingHeader("Terrain Controls", ImGuiTreeNodeFlags_DefaultOpen))
        {
            {
                static int dim = TERRAIN_INIT_SIZE;
                static bool autoUpdate = false;
                static bool falloff = true;
                float tileScale = terrain->tileScale();
                float heightScale = terrain->heightScale();

                if (ImGui::Checkbox(" Falloff Map", &falloff))
                    terrain->enableFalloffMap(falloff);
                // TODO ? with  [vertices^2]
                ImGui::SliderInt("Terrain size", &dim, 4, 512);
                ImGui::SliderFloat("Tile scale", &tileScale, 0.01f, 10.f);
                ImGui::SliderFloat("Height scale", &heightScale, 0.01f, 10.f);

                ImGui::NewLine();
                if (ImGui::Button("Generate"))
                {
                    terrain->setSize(dim, dim);
                    terrain->setTileScale(tileScale);
                    terrain->setHeightScale(heightScale);
                }
                else if (autoUpdate)
                {
                    terrain->setSize(dim, dim);
                    terrain->setTileScale(tileScale);
                    terrain->setHeightScale(heightScale);
                }
                ImGui::SameLine();
                ImGui::Checkbox("Auto", &autoUpdate);
            }
            ImGui::Separator();
            if (ImGui::TreeNodeEx("Noise Map Generation", ImGuiTreeNodeFlags_DefaultOpen))
            {
                static ProceduralTex2D& noiseMap = terrain->heightMap();
                static bool autoUpdate = false;
                int32_t seed = noiseMap.seed();
                float scale = noiseMap.scale();

                if (ImGui::DragInt("Seed", &seed))
                    noiseMap.reseed(seed);
                if (ImGui::DragFloat("Scale", &scale, 0.01f, 0.f, 100.f))
                    noiseMap.setScale(scale);

                // Select noise function
                ImGui::NewLine();
                const char* items[] = { "Random", "Perlin", "Accumulated Perlin"};
                static int noiseType = noiseMap.type();
                ImGui::Combo("Noise function", &noiseType, items, IM_ARRAYSIZE(items));

                if (noiseType == Noise::OctavesPerlin2D)
                {
                    int32_t octaves = noiseMap.octaves();
                    float persistence = noiseMap.persistence();
                    float lacunarity = noiseMap.lacunarity();
                    // TODO (?)
                    if (ImGui::SliderInt("Octaves", &octaves, 1, 32))
                        noiseMap.setOctaves(octaves);
                    if (ImGui::DragFloat("Persistence", &persistence, 0.01f, 0.f, 1.f))
                        noiseMap.setPersistence(persistence);
                    if (ImGui::DragFloat("Lacunarity", &lacunarity, 0.01f, 1.f, 100.f))
                        noiseMap.setLacunarity(lacunarity);
                }

                noiseMap.setType(static_cast<Noise::Type>(noiseType));

                // Visualize the procedural texture
                const float texW = 156;
                const float texH = 156;
                showTexture(noiseMap.ID(), noiseMap.size(), texW, texH);

                // TODO (?)
                if (ImGui::Button("  Apply  "))
                    terrain->applyNoiseMap();
                else if (autoUpdate)
                    terrain->applyNoiseMap();

                ImGui::SameLine();
                ImGui::Checkbox("Auto", &autoUpdate);
                ImGui::TreePop();
            }
            ImGui::Separator();
            if (ImGui::TreeNode("Texturing"))
            {

                static int use_type = 0;
                static int filterType = 0;
                const uint8_t TERR_COLORS = 0;
                const uint8_t TERR_TEXTURES = 1;
                const char* items[] = { "Nearest", "Linear" };

                // Select Filtering
                ImGui::NewLine();
                if (ImGui::Combo("Filtering", &filterType, items, IM_ARRAYSIZE(items)))
                {
                    if (filterType == 0)
                        terrain->setFilteringPoint();
                    else if (filterType == 1)
                        terrain->setFilteringLinear();
                }

                // Select type of texturing
                ImGui::RadioButton("Use colors", &use_type, TERR_COLORS); 
                HelpMarker("Click on the colored square to open a color picker.\n"
                               "Click and hold to use drag and drop.\n"
                               "Right-click on the colored square to show options.\n"
                               "CTRL+click on individual component to input value.\n");
                ImGui::SameLine();
                ImGui::RadioButton("Use textures", &use_type, TERR_TEXTURES);

                // Terrain Colors
                if (use_type == TERR_COLORS)
                {
                    static std::vector<Terrain::Region>& regions = terrain->regions();
                    std::string elemName;

                    ImGui::Text("Name               Height            Color");
                    ImGui::Separator();
                    
                    // Show color regions
                    ImGui::PushItemWidth(122);
                    for (uint32_t i = 0; i < regions.size(); ++i)
                    {
                        Terrain::Region& r = regions[i];
                        elemName = "##";
                        elemName += std::to_string(i);
                        ImGui::InputText(elemName.c_str(), r.name.data(), 16);
                        ImGui::SameLine();
                        elemName += std::to_string(i);
                        if (ImGui::SliderFloat(elemName.c_str(), &r.toHeight, 0.01f, 1.0f))
                            terrain->onRegionsChanged();

                        ImGui::SameLine();
                        elemName += std::to_string(i);
                        if (ImGui::ColorEdit3(elemName.c_str(), (float*)&r.color, 
                                          ImGuiColorEditFlags_NoInputs | 
                                          ImGuiColorEditFlags_NoLabel))
                            terrain->onRegionsChanged();

                        ImGui::Separator();
                    }
                    ImGui::PopItemWidth();
                }
                // Textures
                else if (use_type == TERR_TEXTURES)
                {
                    // TODO scale icons
                    ImGui::TextWrapped("Hello, below are images of textures");
                    
                    const float texW = 96;
                    const float texH = 96;

                    for (int i = 0; i < 3; ++i)
                    {
                      if (i > 0) { ImGui::SameLine(); }
                      showTexture(terrain->heightMap().ID(), terrain->heightMap().size(), 
                                  texW, texH);
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
                    terrain->totalTriangles());

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


void Application::showTexture(uint32_t texture_id, const glm::uvec2& texSize, 
                              const float w, const float h)
{
    ImTextureID tex_id = (void*)(intptr_t)texture_id;
    ImGui::BeginGroup();
        ImGui::Text("%u x %u", texSize.x, texSize.y);
        HelpMarker("Same size as the terrain");
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImVec2 uv_min = ImVec2(0.0f, 0.0f);                 // Top-left
        ImVec2 uv_max = ImVec2(1.0f, 1.0f);                 // Lower-right
        ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
        ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white
        ImGui::Image(tex_id, ImVec2(w, h), uv_min, 
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
            else if (region_x > w - region_sz) { 
                region_x = w - region_sz; }
            if (region_y < 0.0f) { region_y = 0.0f; }
            else if (region_y > h - region_sz) {
                region_y = h - region_sz; }
            ImGui::Text("Min: (%.2f, %.2f)", region_x, region_y);
            ImGui::Text("Max: (%.2f, %.2f)", region_x + region_sz, 
                                             region_y + region_sz);
            ImVec2 uv0 = ImVec2((region_x) / w, (region_y) / h);
            ImVec2 uv1 = ImVec2((region_x + region_sz) / w, 
                                (region_y + region_sz) / h);
            ImGui::Image(tex_id, ImVec2(region_sz * zoom, 
                                           region_sz * zoom), 
                         uv0, uv1, tint_col, border_col);
            ImGui::EndTooltip();
        }
    ImGui::EndGroup();
}

void Application::cameraSetPresetTop()
{
    camera->set_position(glm::vec3(5,20,0));
    camera->set_pitch(-89);
    camera->set_yaw(270);
    camera->set_field_of_view(45);
}

void Application::cameraSetPresetFront()
{
    camera->set_position(glm::vec3(0,5,13));
    camera->set_pitch(-22);
    camera->set_yaw(270);
    camera->set_field_of_view(45);
}

void Application::cameraSetPresetSideWays()
{
    camera->set_position(glm::vec3(7,11,12));
    camera->set_pitch(-40);
    camera->set_yaw(245);
    camera->set_field_of_view(45);
}

