/**
 *  Copyright (c) 2022 ProceduralTerrain authors Distributed under MIT License 
 * (http://opensource.org/licenses/MIT)
 */

#define SGL_DEBUG
#include "ProceduralTerrain.h"
#include "ResourceManager.h"

#include <SGL/opengl/ShaderObject.h>


static void ResizeCallback(GLFWwindow*, int, int);
static void MouseMoveCallback(GLFWwindow*, double, double);
static void MousePressedCallback(GLFWwindow*, int, int, int);
static void KeyPressedCallback(GLFWwindow*, int, int, int, int);

ProceduralTerrain::ProceduralTerrain()
{
    CreateShaders();
    CreateTextures();
    CreateSceneObjects();
}

ProceduralTerrain::~ProceduralTerrain()
{
    ResourceManager::ClearAll();
}

void ProceduralTerrain::CreateShaders()
{
    CreateSkyboxShader();
    CreateTerrainShaders();
}

void ProceduralTerrain::CreateTextures()
{
    LoadTerrainTextures();
}

void ProceduralTerrain::CreateSceneObjects()
{
    CreateCamera();
    CreateSkybox();
    CreateProceduralTexture();
    CreateTerrainColorMap();
    CreateTerrain();
}        

void ProceduralTerrain::Start()    
{
    SetupPreRenderStates();

    m_Window->SetUserPointer(this);

    m_Window->SetWindowSizeCallback(ResizeCallback);
    m_Window->SetCursorPosCallback(MouseMoveCallback);
    m_Window->SetMouseButtonCallback(MousePressedCallback);
    m_Window->SetKeyCallback(KeyPressedCallback);
}

void ProceduralTerrain::SetupPreRenderStates()
{
    m_Window->SetVSync(true);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    CameraSetPresetTop();
}

// =============================================================================

void ProceduralTerrain::Update(float dt)
{
    m_Camera->Update(dt);

    const glm::mat4& kProjMat = m_Camera->GetProjMat();
    const glm::mat4& kViewMat = m_Camera->GetViewMat();
    m_ProjViewMat = kProjMat * kViewMat;
}

void ProceduralTerrain::Render()
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_TerrainShader->Use();

    m_ColorMap->Bind();

    m_TerrainShader->SetMat4("MVP", m_ProjViewMat * glm::mat4(1.0));
    if (!m_RenderWireframe)
        m_Terrain->Render();
    else
    {
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
            m_Terrain->Render();
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    }

    m_Skybox->Render( m_Camera->GetViewMat(),
                      m_Camera->GetProjMat() );
}

void ProceduralTerrain::OnImGuiRender()
{
    if (m_State == State::Modify)
    {
        ShowInterface();
    }

    StatusWindow();
}

// =============================================================================

void ProceduralTerrain::SetStateModify()
{
    m_State = State::Modify;

    // Show the cursor
    glfwSetInputMode(*m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void ProceduralTerrain::SetStateFreeFly()
{
    m_State = State::FreeFly;

    // Hide the cursor
    glfwSetInputMode(*m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

// =============================================================================

void ProceduralTerrain::CreateSkyboxShader()
{
    SGL_FUNCTION();
    const auto vertShader = sgl::ShaderObject::Create(
        sgl::ShaderStage::Vertex,
        sgl::LoadTextFile(s_kSkyboxVS)
    );

    const auto fragShader = sgl::ShaderObject::Create(
        sgl::ShaderStage::Fragment,
        sgl::LoadTextFile(s_kSkyboxFS)
    );

    auto skyboxShader = sgl::Shader::Create({ vertShader, fragShader });

    ResourceManager::AddShader(skyboxShader, s_kSkyboxShaderName);
}

void ProceduralTerrain::CreateTerrainShaders()
{
    SGL_FUNCTION();
    
    const auto vertShader = sgl::ShaderObject::Create(
        sgl::ShaderStage::Vertex,
        sgl::LoadTextFile(s_kTerrainVS)
    );

    const auto fragShader = sgl::ShaderObject::Create(
        sgl::ShaderStage::Fragment,
        sgl::LoadTextFile(s_kTerrainFS)
    );

    m_TerrainShader = sgl::Shader::Create({ vertShader, fragShader });
}

void ProceduralTerrain::LoadTerrainTextures()
{
    SGL_FUNCTION();
    /*
    for (auto texturePath : s_kTerrainTexturePaths)
        ResourceManager::LoadTexture(texturePath, s_kForceRGB, s_kMipmaps);
    */
}

void ProceduralTerrain::CreateCamera()
{
    SGL_FUNCTION();
    m_Camera = std::make_unique<Camera>(
        static_cast<float>( m_Window->GetWidth() ) /
        static_cast<float>( m_Window->GetHeight() )
    );
}

void ProceduralTerrain::CreateSkybox()
{
    SGL_FUNCTION();
    m_Skybox = Skybox::CreateUniq(
        ResourceManager::GetShader(s_kSkyboxShaderName),
        s_kSkyboxTexturePaths
    );
}

void ProceduralTerrain::CreateProceduralTexture()
{
    SGL_FUNCTION();
    m_NoiseMap = ProceduralTexture2D::Create(m_TextureSize);
    m_NoiseMap->SetScale(glm::vec2(32.0f));
    m_NoiseMap->GenerateValues();
}

void ProceduralTerrain::CreateTerrain()
{
    SGL_FUNCTION();
    /*
    std::array<
        std::shared_ptr<sgl::Shader>,
        s_kTerrainShaderNames.size()
    > shaders;

    for (size_t i = 0; i < textures.size(); ++i)
        shaders[i] =
            ResourceManager::GetShader(s_kTerrainShaderNames[i]);

    std::array<
        std::shared_ptr<sgl::Texture2D>,
        s_kTerrainTexturePaths.size()
    > textures;

    for (size_t i = 0; i < textures.size(); ++i)
        textures[i] = 
            ResourceManager::GetTexture(s_kTerrainTexturePaths[i]);
    */

    m_Terrain = Terrain::CreateUniq(
        m_TextureSize,
        m_NoiseMap->GetValues()
    );
}

void ProceduralTerrain::CreateTerrainColorMap()
{
    SGL_FUNCTION();
    FillColorRegionSearchMap();

    std::vector<Color> colors = GenerateColorData();

    m_ColorMap = sgl::Texture2D::Create({
        m_TextureSize.x, m_TextureSize.y,
        colors.data(),
        GL_RGB8, GL_RGB, GL_FLOAT,
        true
    });
}

void ProceduralTerrain::FillColorRegionSearchMap()
{
    for (const auto& region : s_kColorRegions)
    {
        m_ColorRegionSearchMap[region.heightTopBound] = region.color;
    }
}

std::vector<ProceduralTerrain::Color> ProceduralTerrain::GenerateColorData()
{
    std::vector<Color> colors(m_TextureSize.x * m_TextureSize.y);

    for (uint32_t y = 0; y < m_TextureSize.y; ++y)
        for (uint32_t x = 0; x < m_TextureSize.x; ++x)
        {
            const uint32_t kIndex = y*m_TextureSize.x + x;
            const float kHeight = m_NoiseMap->operator[](kIndex);

            const auto colorRangeIt =
                m_ColorRegionSearchMap.lower_bound(kHeight);
            if ( colorRangeIt != m_ColorRegionSearchMap.end() )
                colors[kIndex] = colorRangeIt->second;
            else
                colors[kIndex] = s_kDefaultColor;
        }

    return colors;
}

void ProceduralTerrain::CameraSetPresetTop()
{
    m_Camera->SetPosition(glm::vec3(0,100,0));
    m_Camera->SetPitch(-89);
    m_Camera->SetYaw(180);
    m_Camera->SetFOV(45);
}

void ProceduralTerrain::CameraSetPresetFront()
{
    m_Camera->SetPosition(glm::vec3(0,30,90));
    m_Camera->SetPitch(-20);
    m_Camera->SetYaw(270);
    m_Camera->SetFOV(45);
}

void ProceduralTerrain::CameraSetPresetSideways()
{
    m_Camera->SetPosition(glm::vec3(60,32,65));
    m_Camera->SetPitch(-36);
    m_Camera->SetYaw(230);
    m_Camera->SetFOV(45);
}

static void ResizeCallback(GLFWwindow *w, int width, int height)
{
    auto app = (ProceduralTerrain*)glfwGetWindowUserPointer(w);
    app->OnResize(w, width, height);
}

static void MouseMoveCallback(GLFWwindow *w, double x, double y)
{
    auto app = (ProceduralTerrain*)glfwGetWindowUserPointer(w);
    app->OnMouseMove(w, x, y);
}

static void MousePressedCallback(GLFWwindow *w, int button, int action,
                                 int mods)
{
    auto app = (ProceduralTerrain*)glfwGetWindowUserPointer(w);
    app->OnMousePressed(w, button, action, mods);
}

static void KeyPressedCallback(GLFWwindow *w, int key, int scancode,
                               int action, int mods)
{
    auto app = (ProceduralTerrain*)glfwGetWindowUserPointer(w);
    app->OnKeyPressed(w, key, scancode, action, mods);
}

// =============================================================================

void ProceduralTerrain::OnResize(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void ProceduralTerrain::OnMouseMove(GLFWwindow *window, double x, double y)
{
    if (m_State == State::FreeFly)
        m_Camera->OnMouseMove(x, y);
}

void ProceduralTerrain::OnMousePressed(GLFWwindow *window, int button,
                                       int action, int mods)
{

}

void ProceduralTerrain::OnKeyPressed(GLFWwindow *window, int key, int scancode,
                                     int action, int mods)
{
    m_Camera->OnKeyPressed(key, action);

    if (key == KEY_TOGGLE_MENU && action == GLFW_RELEASE)
    {
        if (m_State == State::Modify)
            SetStateFreeFly();
        else
            SetStateModify();
    }
}
