/**
 *  Copyright (c) 2022 ProceduralTerrain authors Distributed under MIT License 
 * (http://opensource.org/licenses/MIT)
 */

#define SGL_DEBUG
#include "ProceduralTerrain.h"
#include "ResourceManager.h"


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
    CreateProceduralTexture();
    CreateTerrainTextureArray();
}

void ProceduralTerrain::CreateSceneObjects()
{
    CreateCamera();
    CreateSkybox();
    CreateTerrain();
    CreateTerrainUBO();
    CreateLightingUBO();
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
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    m_TexArray->Bind();

    CameraSetPresetTop();
}

// =============================================================================

void ProceduralTerrain::Update(float dt)
{
    m_Camera->Update(dt);

    m_ProjViewMat = m_Camera->GetProjMat() * m_Camera->GetViewMat();
}

void ProceduralTerrain::Render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_TerrainShader->Use();
    m_TerrainShader->SetMat4("MVP", m_ProjViewMat * glm::mat4(1.0));

    UpdateTerrainUBO();
    UpdateLightingUBO();

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
    m_NoiseMap = ProceduralTexture2D::Create(m_TextureSize.x, m_TextureSize.y);
    m_NoiseMap->SetScale(220.0);

    m_NoiseMap->GenerateValues();
    m_NoiseMap->UpdateTexture();
}

std::vector<sgl::STBData> ProceduralTerrain::LoadTerrainTextures() const
{
    std::vector<sgl::STBData> imageData(s_kTerrainTexturePaths.size());

    for (uint32_t i = 0; i < imageData.size(); ++i)
    {
        auto& data = imageData[i];
        data.data = sgl::LoadImageData(s_kTerrainTexturePaths[i],
                                       data.width, data.height,
                                       data.channels);
        SGL_ASSERT(data.Loaded());
    }

    return imageData;
}

void ProceduralTerrain::CreateTerrainTextureArray()
{
    SGL_FUNCTION();

    std::vector<sgl::STBData> imgData = LoadTerrainTextures();

    std::vector<const void*> data(imgData.size());
    for (uint32_t i = 0; i < imgData.size(); ++i)
    {
        SGL_ASSERT(imgData[i].width <= m_TexArrayTexWidth);
        SGL_ASSERT(imgData[i].height <= m_TexArrayTexHeight);
        data[i] = static_cast<const void*>(imgData[i].data);
    }
    
    m_TexArray = std::make_unique<sgl::Texture2DArray>(
        sgl::Texture2DArray::TextureInfo{
            m_TexArrayTexWidth,
            m_TexArrayTexHeight,
            m_TexArrayTexFormat,
            GL_RGB,
            GL_UNSIGNED_BYTE,
            true
        },  data );
}

void ProceduralTerrain::CreateTerrain()
{
    SGL_FUNCTION();
    
    m_Terrain = Terrain::CreateUniq(
        m_TextureSize,
        m_NoiseMap->GetValues()
    );

    m_Terrain->SetTileScale(0.05);
    m_Terrain->SetHeightScale(10.0);
    m_Terrain->Generate();
}

void ProceduralTerrain::CreateTerrainUBO()
{
    m_TerrainUBO = std::make_unique<sgl::UniformBuffer>(
        sizeof(TerrainUBO),
        s_kTerrainUBOBindingPoint
    );
}

void ProceduralTerrain::CreateLightingUBO()
{
    m_LightingUBO = std::make_unique<sgl::UniformBuffer>(
        sizeof(LightingUBO),
        s_kLightingUBOBindingPoint
    );
}

void ProceduralTerrain::CameraSetPresetTop()
{
    const float kHeight = m_Terrain->GetSize().y * m_Terrain->GetTileScale();
    m_Camera->SetPosition(glm::vec3(0, kHeight ,0));
    m_Camera->SetPitch(-89);
    m_Camera->SetYaw(270);
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

void ProceduralTerrain::AddNewRegion(const char* name)
{
    if ( m_Regions.size() >= s_kMaxRegionCount )
        return;
    
    Region region;
    region.name = name;
    region.startHeight = m_Regions.empty() ? 0.0f : m_Regions.back().startHeight;
    region.tint = s_kDefaultColor; // TODO randomize
    region.blendStrength = 0.5;
    region.tintStrength = 1.0;
    region.scale = 1.0;

    m_Regions.push_back(region);
}

void ProceduralTerrain::UpdateTerrainUBO()
{
    //if (!m_TerrainChanged)
        //return;

    const float kTerrainHeightScale = m_Terrain->GetHeightScale();

    m_TerrainUBOData.minHeight = m_NoiseMap->GetMinValue() * kTerrainHeightScale;
    m_TerrainUBOData.maxHeight = m_NoiseMap->GetMaxValue() * kTerrainHeightScale;
    //m_TerrainChanged = false;

    // TODO if regions changed

    m_TerrainUBOData.regionCount = static_cast<int>(m_Regions.size());

    SGL_ASSERT(m_Regions.size() <= s_kMaxRegionCount);

    for (uint32_t i = 0; i < m_Regions.size(); ++i)
    {
        auto& region = m_TerrainUBOData.regions[i];

        region.texScale = m_Regions[i].scale;
        region.texIndex = m_Regions[i].texIndex;
        region.tint = m_Regions[i].tint;
        region.tintStrength = m_Regions[i].tintStrength;
        region.blendStrength = m_Regions[i].blendStrength;
        region.startHeight = m_Regions[i].startHeight;
    }

    // TODO null the rest?

    m_TerrainUBO->SetData( &m_TerrainUBOData, sizeof(TerrainUBO) );
}

void ProceduralTerrain::UpdateLightingUBO() const
{
    m_LightingUBO->SetData( &m_LightingUBOData, sizeof(LightingUBO) );
}
