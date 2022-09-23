/**
 *  Copyright (c) 2022 ProceduralTerrain authors Distributed under MIT License 
 * (http://opensource.org/licenses/MIT)
 */

#pragma once

#include <memory>
#include <unordered_map>
#include <map>

#define SGL_USE_IMGUI
#include <SGL/SGL.h>

#include "scene/Camera.h"
#include "scene/Skybox.h"
#include "scene/ProceduralTexture2D.h"
#include "scene/Terrain.h"


class ProceduralTerrain : public sgl::Application
{
public:
    // TODO app params
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
    void LoadTerrainTextures();

    void CreateCamera();
    void CreateSkybox();
    void CreateProceduralTexture();
    void CreateTerrainColorMap();
    void CreateTerrain();

    void FillColorRegionSearchMap();
    std::vector<Color> GenerateColorData();

    void SetupPreRenderStates();

    void ShowInterface();
    void StatusWindow();

    void SetStateModify();
    void SetStateFreeFly();

    void CameraSetPresetTop();
    void CameraSetPresetFront();
    void CameraSetPresetSideways();

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

    glm::uvec2 m_TextureSize{ 512 };

    std::shared_ptr<ProceduralTexture2D> m_NoiseMap;
    std::shared_ptr<sgl::Texture2D> m_ColorMap;

    std::unique_ptr<Terrain> m_Terrain;
    std::shared_ptr<sgl::Shader> m_TerrainShader;

    bool m_RenderWireframe{ false };

    // -------------------------------------------------------------------------
    // Coloring

    struct ColorRegion
    {
        const char* name;
        float heightTopBound;
        Color color;
    };

    // TODO try constructors
    static constexpr std::array s_kColorRegions{
        ColorRegion{"Water Deep",    0.3,  Color(0.0, 0.0, 0.8)       },
        ColorRegion{"Water Shallow", 0.4,  Color(54, 103, 199)/255.f  },
        ColorRegion{"Sand",          0.45, Color(210, 208, 125)/255.f },
        ColorRegion{"Grass",         0.55, Color(86, 152, 23)/255.f   },
        ColorRegion{"Trees",         0.6,  Color(62, 107, 18)/255.f   },
        ColorRegion{"Rock",          0.7,  Color(90, 69, 60)/255.f    },
        ColorRegion{"Higher Rock",   0.9,  Color(75, 60, 53)/255.f    },
        ColorRegion{"Snow",          1.0,  Color(1.0, 1.0, 1.0)       }
    };
    static constexpr glm::vec3 s_kDefaultColor{1.0};

    // Colors per height range
    std::map<float, glm::vec3> m_ColorRegionSearchMap;


private:
    // -------------------------------------------------------------------------
    // Assets
    #define PREFIX ""

    // Shaders
    static constexpr auto s_kSkyboxVS = PREFIX "shaders/SkyboxVS.vs",
                          s_kSkyboxFS = PREFIX "shaders/SkyboxFS.fs",
                          s_kSkyboxShaderName = "skybox";

    static constexpr auto s_kTerrainVS = PREFIX "shaders/TerrainVS.vs",
                          s_kTerrainFS = PREFIX "shaders/TerrainFS.fs",
                          s_kTerrainShaderName = "terrain";

    static constexpr Skybox::FacesPaths s_kSkyboxTexturePaths {
        PREFIX "textures/skybox/right.jpg",
        PREFIX "textures/skybox/left.jpg",
        PREFIX "textures/skybox/top.jpg",
        PREFIX "textures/skybox/bottom.jpg",
        PREFIX "textures/skybox/front.jpg",
        PREFIX "textures/skybox/back.jpg"
    };
};

