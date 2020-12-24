/**********************************************************
 * < Procedural Terrain Generator >
 * @author Martin Smutny, kentril.despair@gmail.com
 * @date 20.12.2020
 * @file application.cpp
 * @brief Main application abstraction
 *********************************************************/

#include "pch.hpp"
#include "application.hpp"

// --------------------------------------------------------------------------
// Static variables
// --------------------------------------------------------------------------


// --------------------------------------------------------------------------
Application::Application(GLFWwindow* w, size_t initial_width, size_t initial_height) 
  : m_window(w),
    m_width(initial_width), 
    m_height(initial_height),
    m_state(STATE_MODIFY)
{
    LOG_INFO("Screen Dimensions: " << m_width << " x " << m_height);

    // "Show" the cursor
    glfwSetInputMode(w, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // --------------------------------------------------------------------------
    // TODO initialize TODO
    // --------------------------------------------------------------------------
    m_camera = std::make_unique<Camera>(float(m_width) / float(m_height));
    cameraSetPresetSideWays();
    
    std::shared_ptr<Shader> shSkybox = std::make_shared<Shader>(
                                         "shaders/draw_skybox.vs", 
                                         "shaders/draw_skybox.fs");
    m_skybox = std::make_unique<Skybox>(shSkybox, 
                                        std::initializer_list<const char*>{
                                         "images/skybox/right.jpg",
                                         "images/skybox/left.jpg",
                                         "images/skybox/top.jpg",
                                         "images/skybox/bottom.jpg",
                                         "images/skybox/front.jpg",
                                         "images/skybox/back.jpg"});

    std::shared_ptr<Shader> shTerSingle = std::make_shared<Shader>(
                                          "shaders/draw_terrain.vs",
                                          "shaders/draw_terrain.fs");
    std::shared_ptr<Shader> shTerMulti = std::make_shared<Shader>(
                                          "shaders/terrain_multitex.vs",
                                          "shaders/terrain_multitex.fs");
    std::shared_ptr<Shader> shTerShadingSingle = std::make_shared<Shader>(
                                          "shaders/draw_terrain_shaded.vs",
                                          "shaders/draw_terrain_shaded.fs");
    std::shared_ptr<Shader> shTerShadingMulti = std::make_shared<Shader>(
                                          "shaders/terrain_multitex_shaded.vs",
                                          "shaders/terrain_multitex_shaded.fs");
    m_terrain = std::make_unique<Terrain>(TERRAIN_INIT_SIZE, TERRAIN_INIT_SIZE, 
                                          0.2f, 2.f);
    m_terrain->addShader(shTerSingle);
    m_terrain->addShader(shTerMulti);
    m_terrain->addShader(shTerShadingSingle);
    m_terrain->addShader(shTerShadingMulti);

    // Load textures
    m_terrain->loadTexture("images/waterDeep.jpg");
    m_terrain->loadTexture("images/water.jpg");
    m_terrain->loadTexture("images/sand.jpg");
    m_terrain->loadTexture("images/grass.jpg");
    m_terrain->loadTexture("images/trees.jpg");
    m_terrain->loadTexture("images/rocks.jpg");
    m_terrain->loadTexture("images/rocksHigh.jpg");
    m_terrain->loadTexture("images/snow.jpg");
    m_terrain->initTextureArray();

    // --------------------------------------------------------------------------
    // Register callbacks
    // --------------------------------------------------------------------------
    m_callbackMap = {
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
    m_lastFrame = glfwGetTime();
    m_framestamp = m_lastFrame;
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
    m_deltaTime = currentFrame - m_lastFrame;
    m_lastFrame = currentFrame;

    // Frametime and FPS counter, updates once per 1 second
    if (currentFrame - m_framestamp > 1.0f)
    {
        m_framestamp += 1.0f;
        m_frames = 0;
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
    glViewport(0, 0, m_width, m_height);

    // --------------------------------------------------------------------------
    // Sart the Dear ImGUI frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    // --------------------------------------------------------------------------
    // Draw the scene
    // --------------------------------------------------------------------------
    // Get projection and view matrices defined by camera
    glm::mat4 proj = m_camera->proj_matrix();
    glm::mat4 view = m_camera->view_matrix();
    m_projView = proj * view;

    // Render the terrain
    if (!m_wireframe)
        m_terrain->render(m_projView, m_camera->position());
    else
    {
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
            m_terrain->render(m_projView, m_camera->position());
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    }

    // Render skybox as last
    m_skybox->render(view, proj);

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

    m_frames++;
}

void Application::update()
{
    m_camera->update(m_deltaTime);
}


void Application::on_resize(GLFWwindow *window, int width, int height)
{
    m_width = width;
    m_height = height;
    DERR("SCREEN RESIZE");
}

void Application::on_mouse_move(GLFWwindow *window, double x, double y) 
{ 
    if (m_state == STATE_FREEFLY)
        m_camera->on_mouse_move(x, y);
}

void Application::on_mouse_pressed(GLFWwindow *window, int button, int action, int mods)
{

}

void Application::on_key_pressed(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    m_key = key;
    m_keyAction = action;

    call_registered(key, action);
}

/**@brief ImGui: adds "(?)" with hover one the same line as the prev obj */
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
    if (m_state == STATE_MODIFY)
    {
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

            ImGui::Checkbox(" Wireframe", &m_wireframe);
        }

        if (ImGui::CollapsingHeader("Camera Settings"))
        {
            // TODO help (?)
            static float fov = glm::radians(m_camera->field_of_view());
            static float nearC = m_camera->near_plane_dist();
            static float farC = m_camera->far_plane_dist();

            glm::vec3 pos = m_camera->position();
            float pitch = m_camera->pitch();
            float yaw = m_camera->yaw();

            if (ImGui::InputFloat3("Position", glm::value_ptr(pos)))
                m_camera->set_position(pos);
            if (ImGui::SliderFloat("Pitch angle", &pitch, -89.f, 89.f, "%.0f deg"))
                m_camera->set_pitch(pitch);
            if (ImGui::SliderFloat("Yaw angle", &yaw, 0.f, 360.f, "%.0f deg"))
                m_camera->set_yaw(yaw);
            if (ImGui::SliderAngle("Field of view", &fov, 0.f, 180.f))
                m_camera->set_field_of_view(fov);
            if (ImGui::SliderFloat("Near plane", &nearC, 0.f, 10.f))
                m_camera->set_near_plane_dist(nearC);
            if (ImGui::SliderFloat("Far plane", &farC, 100.f, 1000.f))
                m_camera->set_far_plane_dist(farC);

            // TODO camera preset positions relative to terrain size
            ImGui::Text("Position Presets");
            ImGui::Separator();
            if (ImGui::Button("From Top")) { cameraSetPresetTop(); }
            ImGui::SameLine();
            if (ImGui::Button("From Front")) { cameraSetPresetFront(); }
            ImGui::SameLine();
            if (ImGui::Button("From Side")) { cameraSetPresetSideWays(); }

            ImGui::NewLine();
        }
        if (ImGui::CollapsingHeader("Terrain Controls", ImGuiTreeNodeFlags_DefaultOpen))
        {
            {
                static int dim = TERRAIN_INIT_SIZE;
                static bool autoUpdate = false;
                static bool falloff = true;
                static float tileScale = m_terrain->tileScale();
                static float heightScale = m_terrain->heightScale();

                if (ImGui::Checkbox(" Falloff Map", &falloff))
                    m_terrain->enableFalloffMap(falloff);

                // TODO ? with  [vertices^2]
                ImGui::SliderInt("Terrain size", &dim, 4, 512);
                ImGui::SliderFloat("Tile scale", &tileScale, 0.01f, 10.f);
                ImGui::SliderFloat("Height scale", &heightScale, 0.01f, 10.f);

                ImGui::NewLine();
                if (ImGui::Button("Generate"))
                {
                    m_terrain->setSize(dim, dim);
                    m_terrain->setTileScale(tileScale);
                    m_terrain->setHeightScale(heightScale);
                }
                else if (autoUpdate)
                {
                    m_terrain->setSize(dim, dim);
                    m_terrain->setTileScale(tileScale);
                    m_terrain->setHeightScale(heightScale);
                }
                ImGui::SameLine();
                ImGui::Checkbox("Auto", &autoUpdate);
            }
            ImGui::Separator();
            if (ImGui::TreeNodeEx("Noise Map Generation", ImGuiTreeNodeFlags_DefaultOpen))
            {
                static ProceduralTex2D& noiseMap = m_terrain->heightMap();
                static bool autoUpdate = false;
                int32_t seed = noiseMap.seed();
                float scale = noiseMap.scale();

                if (ImGui::DragInt("Seed", &seed))
                    noiseMap.reseed(seed);
                if (ImGui::DragFloat("Scale", &scale, 0.01f, 0.f, 100.f))
                    noiseMap.setScale(scale);

                // Select noise function
                ImGui::NewLine();
                const char* noiseTypes[] = { "Random", "Perlin", "Accumulated Perlin"};
                static int noiseType = noiseMap.type();
                ImGui::Combo("Noise function", &noiseType, noiseTypes, 
                             IM_ARRAYSIZE(noiseTypes));

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
                    m_terrain->applyNoiseMap();
                else if (autoUpdate)
                    m_terrain->applyNoiseMap();

                ImGui::SameLine();
                ImGui::Checkbox("Auto", &autoUpdate);
                ImGui::TreePop();
            }
            ImGui::Separator();
            if (ImGui::TreeNode("Texturing"))
            {
                static int useType = 0;
                static int filterType = 0;
                static bool usingBlending = false;
                const uint8_t TERR_COLORS = 0;
                const uint8_t TERR_TEXTURES = 1;
                const char* filterTypes[] = { "Nearest", "Linear" };

                ImGui::NewLine();
                if (ImGui::Checkbox(" Blending (height-based)", &usingBlending))
                    m_terrain->setBlending(usingBlending);

                // Select Filtering
                if (ImGui::Combo("Filtering", &filterType, filterTypes, 
                                 IM_ARRAYSIZE(filterTypes)))
                {
                    if (filterType == 0)
                        m_terrain->setFilteringPoint();
                    else if (filterType == 1)
                        m_terrain->setFilteringLinear();
                }
                const float calcWidth = ImGui::CalcItemWidth() - 90;

                // Select type of texturing
                ImGui::RadioButton("Use colors", &useType, TERR_COLORS); 
                HelpMarker("Click on the colored square to open a color picker.\n"
                               "Click and hold to use drag and drop.\n"
                               "Right-click on the colored square to show options.\n"
                               "CTRL+click on individual component to input value.\n");
                ImGui::SameLine();
                ImGui::RadioButton("Use textures", &useType, TERR_TEXTURES);

                // Terrain Colors
                if (useType == TERR_COLORS)
                {
                    m_terrain->setUsingColors(true);
                    static std::vector<Terrain::Region>& regions = m_terrain->regions();

                    ImGui::Text("Name               Height            Color");
                    ImGui::Separator();
                    
                    // Show color regions
                    ImGui::PushItemWidth(122);
                    for (uint32_t i = 0; i < regions.size(); ++i)
                    {
                        Terrain::Region& r = regions[i];
                        std::string elemName("##");
                        elemName += std::to_string(i);
                        ImGui::InputText(elemName.c_str(), r.name.data(), 16);
                        ImGui::SameLine();
                        elemName += std::to_string(i);
                        if (ImGui::SliderFloat(elemName.c_str(), &r.toHeight, 0.01f, 1.0f))
                            m_terrain->onRegionsChanged();

                        ImGui::SameLine();
                        elemName += std::to_string(i);
                        if (ImGui::ColorEdit3(elemName.c_str(), (float*)&r.color, 
                                          ImGuiColorEditFlags_NoInputs | 
                                          ImGuiColorEditFlags_NoLabel))
                            m_terrain->onRegionsChanged();

                        ImGui::Separator();
                    }
                    ImGui::PopItemWidth();
                }
                // Textures
                else if (useType == TERR_TEXTURES)
                {
                    float scaleFactor = m_terrain->getScaleFactor();
                    m_terrain->setUsingColors(false);
                    static const std::vector<std::unique_ptr<Texture2D>>& guiTexs 
                        = m_terrain->textures();    // TODO fixed palette

                    ImGui::SetNextItemWidth(calcWidth);
                    if (ImGui::DragFloat("Texture scale", &scaleFactor, 0.01f, 0.1f, 16.f))
                        m_terrain->setScaleFactor(scaleFactor);
                    ImGui::SameLine();
                    if (ImGui::Button("Default"))
                        m_terrain->setScaleFactor(1.0f);
                    
                    // Preview of textures
                    const float texW = 96;
                    const float texH = 96;
                    for (uint32_t j = 0; j < guiTexs.size(); ++j)
                    {
                          if (j > 0 && j % 3 != 0) { ImGui::SameLine(); }
                          showTexture(guiTexs[j]->ID(), guiTexs[j]->size(), 
                                      texW, texH);
                    }
                }
            // Slope controls TODO
                ImGui::TreePop();
            }
            ImGui::Separator();
            if (ImGui::TreeNode("Shading"))
            {
                static bool usingShading = false;
                if (ImGui::Checkbox(" Shading ", &usingShading))
                    m_terrain->setShading(usingShading);

                ImGui::Text("Directional Light");
                ImGui::Separator();
                static struct Terrain::DirLight dirLight = m_terrain->dirLight();
                static glm::vec3& lightDir = dirLight.direction;
                static glm::vec3& lightAmbient = dirLight.ambient;
                static glm::vec3& lightDiffuse = dirLight.diffuse;
                static glm::vec3& lightSpecular = dirLight.specular;

                ImGui::DragFloat3("Direction", glm::value_ptr(lightDir), 0.1f);
                ImGui::ColorEdit3("Ambient",  glm::value_ptr(lightAmbient), 
                                              ImGuiColorEditFlags_Float);
                ImGui::ColorEdit3("Diffuse",  glm::value_ptr(lightDiffuse), 
                                              ImGuiColorEditFlags_Float);
                ImGui::ColorEdit3("Specular", glm::value_ptr(lightSpecular), 
                                              ImGuiColorEditFlags_Float);

                m_terrain->setDirLight(dirLight);

                ImGui::Text("Material");
                ImGui::Separator();

                ImGui::TreePop();
            }
        }
        /*
        if (ImGui::CollapsingHeader("Sky effects"))
        {
            static glm::vec3 light_col(1.0f);
            static glm::vec3 fog_col(1.0f);
            static float fog_falloff = 1.0f;

            ImGui::ColorEdit3("Light color", glm::value_ptr(light_col));
            ImGui::ColorEdit3("Fog color", glm::value_ptr(fog_col));
            ImGui::SliderFloat("Fog falloff dist", &fog_falloff, 1.f, 1000.f);
        }
        */
        ImGui::End();
    }

    status_window();
}

void Application::status_window()
{
    // Overlay when flying with camera
    if (m_state == STATE_FREEFLY)
        ImGui::SetNextWindowBgAlpha(0.35f);

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
                m_terrain->totalVertices(), m_terrain->totalIndices(),
                m_terrain->totalTriangles());

    ImGui::End();
}

void Application::set_state_modify()
{
    set_state(STATE_MODIFY);

    // Show the cursor
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void Application::set_state_freefly()
{
    set_state(STATE_FREEFLY);

    // Hide the cursor
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
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
        //HelpMarker("Same size as the terrain");
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
    m_camera->set_position(glm::vec3(5,20,0));
    m_camera->set_pitch(-89);
    m_camera->set_yaw(270);
    m_camera->set_field_of_view(45);
}

void Application::cameraSetPresetFront()
{
    m_camera->set_position(glm::vec3(0,5,13));
    m_camera->set_pitch(-22);
    m_camera->set_yaw(270);
    m_camera->set_field_of_view(45);
}

void Application::cameraSetPresetSideWays()
{
    m_camera->set_position(glm::vec3(7,11,12));
    m_camera->set_pitch(-40);
    m_camera->set_yaw(245);
    m_camera->set_field_of_view(45);
}

