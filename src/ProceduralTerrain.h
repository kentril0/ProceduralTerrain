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
    void CreateTerrain();

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

    glm::uvec2 m_TerrainSize{ 128 };
    std::shared_ptr<ProceduralTexture2D> m_HeightMap;
    std::unique_ptr<Terrain> m_Terrain;

    bool m_RenderWireframe{ false };

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

