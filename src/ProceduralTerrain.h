/**
 *  Copyright (c) 2022 ProceduralTerrain authors Distributed under MIT License 
 * (http://opensource.org/licenses/MIT)
 */

#pragma once

#include <memory>
#include <unordered_map>

#define SGL_USE_IMGUI
#include <SGL/SGL.h>

#include "scene/Camera.h"
#include "scene/Skybox.h"
#include "scene/ProceduralTexture2D.h"
#include "scene/Terrain.h"


class ProceduralTerrain : public sgl::Application
{
public:
    ProceduralTerrain();
    ~ProceduralTerrain();

    void OnResize(GLFWwindow*, int, int);
    void OnMouseMove(GLFWwindow*, double, double);
    void OnMousePressed(GLFWwindow*, int, int, int);
    void OnKeyPressed(GLFWwindow*, int, int, int, int);

protected:
    virtual void Start() override;
    virtual void Update(float dt) override;
    virtual void Render() override;
    virtual void OnImGuiRender() override;

private:
    using Color = glm::vec3;

    void CreateShaders();
    void CreateTextures();
    void CreateSceneObjects();

    void CreateSkyboxShader();
    void CreateTerrainShaders();

    void LoadSkyboxTextures();
    std::vector<sgl::STBData> LoadTerrainTextures() const;

    void CreateCamera();
    void CreateSkybox();
    void CreateProceduralTexture();
    void CreateTerrainTextureArray();
    void CreateTerrain();

    void SetupPreRenderStates();

    void ShowInterface();
    void StatusWindow();

    void SetStateModify();
    void SetStateFreeFly();

    void CameraSetPresetTop();
    void CameraSetPresetFront();
    void CameraSetPresetSideways();

    void CreateTerrainUBO();
    void CreateLightingUBO();
    void UpdateTerrainUBO();
    void UpdateLightingUBO();

private:
    enum class State
    {
        Modify,
        FreeFly
    };

    enum Controls
    {
        KEY_TOGGLE_MENU  = GLFW_KEY_ESCAPE,
        KEY_CAM_RCURSOR  = KEY_TOGGLE_MENU
    };

    State m_State{ State::Modify };

    std::unique_ptr<Camera> m_Camera;
    glm::mat4 m_ProjViewMat{ 1.0 };

    std::unique_ptr<Skybox> m_Skybox;

    std::shared_ptr<ProceduralTexture2D> m_NoiseMap;
    glm::uvec2 m_TextureSize{ 512 };

    std::unique_ptr<Terrain> m_Terrain;
    std::shared_ptr<sgl::Shader> m_TerrainShader;

    std::unique_ptr<sgl::Texture2DArray> m_TexArray;
    int32_t m_TexArrayTexWidth = 512;
    int32_t m_TexArrayTexHeight = 512;
    uint32_t m_TexArrayTexFormat = GL_RGB8;

    bool m_RenderWireframe{ false };

    // -------------------------------------------------------------------------
    // Terrain Regions

    /** @brief Correspond to s_kTexturePaths array */
    enum TextureIndices {
        TEX_WATER,
        TEX_SAND,
        TEX_GRASS,
        TEX_STONY_GRASS,
        TEX_ROCKY,
        TEX_MOUNTAINS,
        TEX_SNOW
    };

    struct Region
    {
        float startHeight;
        std::string name;
        Color tint;
        int texIndex;      ///< Index into texture paths
        float scale;
        float tintStrength;
        float blendStrength;

        Region();
        Region(int id);
        Region(const std::string& name);
        Region(float startHeight, const std::string& name, Color tint,
               int texIndex, float scale, float tintStrength,
               float blendStrength);
    };

    std::vector<Region> m_Regions{
        { 0.0, "Water Deep", Color(0.0, 0.0, 0.8), TEX_WATER, 2.0, 0.1, 0.2 },
        { 0.1, "Water Shallow", Color(54, 103, 199)/255.f, TEX_WATER, 2.0, 0.1, 0.2 },
        { 0.15, "Sand", Color(210, 208, 125)/255.f, TEX_SAND, 2.0, 0.1, 0.2 },
        { 0.2, "Grass", Color(86, 152, 23)/255.f, TEX_GRASS, 2.0, 0.1, 0.2 },
        { 0.3, "Trees", Color(62, 107, 18)/255.f, TEX_STONY_GRASS, 2.0, 0.1, 0.2 },
        { 0.6, "Rock", Color(90, 69, 60)/255.f, TEX_ROCKY, 2.0, 0.1, 0.2 },
        { 0.8, "Higher Rock", Color(75, 60, 53)/255.f, TEX_MOUNTAINS, 2.0, 0.1, 0.2 },
        { 0.9, "Snow", Color(1.0, 1.0, 1.0), TEX_SNOW, 2.0, 0.1, 0.2 },
    };

    static constexpr Color s_kDefaultColor{ 1.0 };

    // -------------------------------------------------------------------------
    // Uniform Buffers

    static const uint32_t s_kMaxRegionCount = 8;

    struct RegionUBO
    {
        float texScale;
        int texIndex;
        float blendStrength;
        float startHeight;
        glm::vec3 tint;
        float tintStrength;
    };

    struct TerrainUBO
    {
        float minHeight;
        float maxHeight;
        int regionCount;
        alignas(16) RegionUBO regions[s_kMaxRegionCount];
    };

    TerrainUBO m_TerrainUBOData;
    std::unique_ptr<sgl::UniformBuffer> m_TerrainUBO;
    bool m_TerrainChanged{ true };
    bool m_NoiseMapChanged{ false };

    struct LightingUBO
    {
        glm::vec3 sunColor;
        float sunIntensity;
        alignas(16) glm::vec3 sunDir;
        alignas(16) glm::vec3 skyColor;
        alignas(16) glm::vec3 bounceColor;
    };

    LightingUBO m_LightingUBOData{
        glm::vec3(0.7, 0.45, 0.3),
        2.2,
        glm::vec3(0.8, 0.4, 0.2),
        glm::vec3(0.5, 0.8, 0.9),
        glm::vec3(0.7, 0.3, 0.2),
    };
    std::unique_ptr<sgl::UniformBuffer> m_LightingUBO;
    bool m_LightingOptionsChanged{ true };

    static constexpr uint32_t s_kTerrainUBOBindingPoint = 1;
    static constexpr uint32_t s_kLightingUBOBindingPoint = 2;

private:
    // -------------------------------------------------------------------------
    // Assets
    #define PREFIX ""

    // Shaders
    static constexpr auto s_kSkyboxVS = PREFIX "shaders/Skybox.vert",
                          s_kSkyboxFS = PREFIX "shaders/Skybox.frag",
                          s_kSkyboxShaderName = "skybox";

    static constexpr auto s_kTerrainVS = PREFIX "shaders/Terrain.vert",
                          s_kTerrainFS = PREFIX "shaders/Terrain.frag",
                          s_kTerrainShaderName = "terrain";

    static constexpr Skybox::FacesPaths s_kSkyboxTexturePaths {
        PREFIX "textures/skybox/right.jpg",
        PREFIX "textures/skybox/left.jpg",
        PREFIX "textures/skybox/top.jpg",
        PREFIX "textures/skybox/bottom.jpg",
        PREFIX "textures/skybox/front.jpg",
        PREFIX "textures/skybox/back.jpg"
    };

    static constexpr std::array s_kTerrainTexturePaths{
        PREFIX "textures/water.jpg",
        PREFIX "textures/sand.jpg",
        PREFIX "textures/grass.jpg",
        PREFIX "textures/stonyGrass.jpg",
        PREFIX "textures/rocky.jpg",
        PREFIX "textures/mountains.jpg",
        PREFIX "textures/snow.jpg"
    };

};

