/**
 *  Copyright (c) 2022 ProceduralTerrain authors Distributed under MIT License 
 * (http://opensource.org/licenses/MIT)
 */

#include "ProceduralTerrain.h"
#include <glm/gtc/type_ptr.hpp>


/**@brief ImGui: adds "(?)" with hover one the same line as the prev obj */
static void HelpMarker(const char* desc);

static void ShowTexture(uint32_t texture_id, const glm::uvec2& texSize, 
                              const float w, const float h);

void ProceduralTerrain::ShowInterface()
{
    if (!ImGui::Begin("Procedural Terrain Controls", NULL))
    {
        ImGui::End();
        return;
    }

    if (ImGui::CollapsingHeader("Configuration"))
    {
        static bool vsync = true;
        if (ImGui::Checkbox(" Vertical sync", &vsync))
            m_Window->SetVSync(vsync);

        ImGui::Checkbox(" Wireframe", &m_RenderWireframe);
    }

    if (ImGui::CollapsingHeader("Camera Settings"))
    {
        static float fov = glm::radians(m_Camera->GetFOV());
        static float nearDist = m_Camera->GetNearDist();
        static float farDist = m_Camera->GetFarDist();

        glm::vec3 pos = m_Camera->GetPosition();
        float pitch = m_Camera->GetPitch();
        float yaw = m_Camera->GetYaw();

        if (ImGui::DragFloat3("Position", glm::value_ptr(pos)))
            m_Camera->SetPosition(pos);
        if (ImGui::SliderFloat("Pitch angle", &pitch, -89.f, 89.f, "%.0f deg"))
            m_Camera->SetPitch(pitch);
        if (ImGui::SliderFloat("Yaw angle", &yaw, 0.f, 360.f, "%.0f deg"))
            m_Camera->SetYaw(yaw);
        if (ImGui::SliderAngle("Field of view", &fov, 0.f, 180.f))
            m_Camera->SetFOV(fov);
        if (ImGui::SliderFloat("Near plane", &nearDist, 0.f, 10.f))
            m_Camera->SetNearDist(nearDist);
        if (ImGui::SliderFloat("Far plane", &farDist, 100.f, 1000.f))
            m_Camera->SetFarDist(farDist);

        // TODO camera preset positions relative to terrain size
        ImGui::Text("Position Presets");
        ImGui::Separator();
        if (ImGui::Button("From Top")) { CameraSetPresetTop(); }
        ImGui::SameLine();
        if (ImGui::Button("From Front")) { CameraSetPresetFront(); }
        ImGui::SameLine();
        if (ImGui::Button("From Side")) { CameraSetPresetSideways(); }

        ImGui::NewLine();
    }

    if ( ImGui::CollapsingHeader("Terrain Controls", 
                                 ImGuiTreeNodeFlags_DefaultOpen) )
    {
        {
            static bool terrainChanged = false;
            static bool autoUpdate = false;
            static bool falloff = true;
            
            static int terrainSize = m_Terrain->GetSize().x;
            static int terrainLastSize = terrainSize;
            static float tileScale = m_Terrain->GetTileScale();
            static float heightScale = m_Terrain->GetHeightScale();

            terrainChanged |= ImGui::SliderInt("Terrain size", &terrainSize, 4, 2048);
            terrainChanged |= ImGui::SliderFloat("Tile scale", &tileScale, 0.01f, 1.f);
            terrainChanged |= ImGui::SliderFloat("Height scale", &heightScale, 1.f, 32.f);

            ImGui::NewLine();
            const bool kGeneratePressed = ImGui::Button("Generate");
            // TODO if noiseMapChanged as well
            if ( terrainChanged && (kGeneratePressed || autoUpdate) )
            {
                m_TerrainChanged = true;
                if (terrainSize != terrainLastSize)
                {
                    m_NoiseMap->SetSize(glm::uvec2(terrainSize, terrainSize));
                    m_NoiseMap->GenerateValues();
                    m_Terrain->SetSize(glm::uvec2(terrainSize, terrainSize));
                    terrainLastSize = terrainSize;
                }

                m_Terrain->SetTileScale(tileScale);
                m_Terrain->SetHeightScale(heightScale);
                m_Terrain->Generate();

                terrainChanged = false;
            }
            ImGui::SameLine();
            ImGui::Checkbox("Auto", &autoUpdate);
        }
        ImGui::Separator();

        if (ImGui::TreeNodeEx("Noise Map Generation", ImGuiTreeNodeFlags_DefaultOpen))
        {
            static bool noiseMapChanged = false;
            static bool autoUpdate = false;
            static float scale = m_NoiseMap->GetScale();
            static int32_t seed = m_NoiseMap->GetSeed();
            static float offset = m_NoiseMap->GetOffset();
            static int octaves = m_NoiseMap->GetOctaves();
            static float gain = m_NoiseMap->GetGain();
            static float lacunarity = m_NoiseMap->GetLacunarity();

            noiseMapChanged |= ImGui::DragInt("Seed", &seed);
            noiseMapChanged |= ImGui::DragFloat("Scale", &scale, 0.1f, 0.001f, 100.f);
            noiseMapChanged |= ImGui::DragFloat("Offset", &offset, 1.0f, -10000.f, 10000.f);
            // TODO (?)
            noiseMapChanged |= ImGui::SliderInt("Octaves", &octaves, 1, 32);
            noiseMapChanged |= ImGui::DragFloat("Gain (Persistence)", &gain, 0.01f, 0.f, 1.f);
            noiseMapChanged |= ImGui::DragFloat("Lacunarity", &lacunarity, 0.01f, 1.f, 100.f);

            if (noiseMapChanged)
            {
                m_NoiseMap->SetSeed(seed);
                m_NoiseMap->SetOctaves(octaves);
                m_NoiseMap->SetScale(scale);
                m_NoiseMap->SetOffset(offset);
                m_NoiseMap->SetGain(gain);
                m_NoiseMap->SetLacunarity(lacunarity);
            }

            ImGui::NewLine();
            bool kUpdatePressed = ImGui::Button("Update");
            ImGui::SameLine();
            ImGui::Checkbox("Auto", &autoUpdate);

            if ( noiseMapChanged && ( kUpdatePressed || autoUpdate ) )
            {
                m_NoiseMap->GenerateValues();
                m_NoiseMap->UpdateTexture();
                noiseMapChanged = false;
            }

            ShowTexture(m_NoiseMap->GetTexture()->GetID(), m_NoiseMap->GetSize(), 128, 128);

            ImGui::TreePop();
        }
        ImGui::Separator();
        
        ImGui::Separator();
        if (ImGui::TreeNodeEx("Regions", ImGuiTreeNodeFlags_DefaultOpen))
        {
            // TODO global settings

            static bool addRegionState = false;
            static char inputRegionName[65];

            if (!addRegionState)
            {
                if ( ImGui::Button("Add Region") )
                    addRegionState = true;
            }
            else
            {
                ImGui::InputText("Name", inputRegionName,
                                 IM_ARRAYSIZE(inputRegionName) );
                if ( ImGui::Button("Add Region") )
                {
                    AddNewRegion(inputRegionName);
                    inputRegionName[0] = '\0';
                    addRegionState = false;
                }
            }

            for (uint32_t i = 0; i < m_Regions.size(); ++i)
            {
                auto& region = m_Regions[i];

                ImGui::PushID(i);
                if ( ImGui::TreeNode(region.name.c_str()) )
                {
                    ImGui::DragFloat("Start Height", &region.startHeight, 0.01f, 0.0f, 1.0f);
                    ImGui::ColorEdit3("Tint", glm::value_ptr(region.tint));
                    ImGui::Combo("Texture", &region.texIndex,
                                 s_kTerrainTexturePaths.data(),
                                 s_kTerrainTexturePaths.size());
                    ImGui::DragFloat("Tint Strength", &region.tintStrength, 0.01f, 0.0f, 1.0f);
                    ImGui::DragFloat("Blend range", &region.blendStrength, 0.01f, 0.0f, 1.0f);
                    ImGui::DragFloat("Texture Scale", &region.scale, 0.1f, 0.0f, 100.0f);
                    ImGui::TreePop();
                }
                ImGui::PopID();
            }

            ImGui::Separator();
            ImGui::TreePop();
        }

        if (ImGui::TreeNodeEx("Lighting" ))
        {
            auto& data = m_LightingUBOData;
            ImGui::DragFloat3("Sun Direction", glm::value_ptr(data.sunDir), 0.1f);
            ImGui::DragFloat("Sun Intensity", &data.sunIntensity, 0.01f, 0.0f, 5.0f);
            ImGui::ColorEdit3("Sun Color", glm::value_ptr(data.sunColor),
                              ImGuiColorEditFlags_Float);
            ImGui::ColorEdit3("Sky Color", glm::value_ptr(data.skyColor),
                              ImGuiColorEditFlags_Float);
            ImGui::ColorEdit3("Bounce Color", glm::value_ptr(data.bounceColor),
                              ImGuiColorEditFlags_Float);
            ImGui::Separator();
            ImGui::TreePop();
        }
    }
    
    ImGui::End();
}

void ProceduralTerrain::StatusWindow()
{
    // Overlay when flying with camera
    if (m_State == State::FreeFly)
        ImGui::SetNextWindowBgAlpha(0.35f);

    // Collapsed or Clipped
    if (!ImGui::Begin("ProceduralTerrain Metrics", NULL))
    {
        ImGui::End();
        return;
    }

    ImGuiIO& io = ImGui::GetIO();
    // frametime and FPS
    ImGui::Text("ProceduralTerrain average %.3f ms/frame (%.1f FPS)", 
                1000.0f / io.Framerate, io.Framerate);
    ImGui::Text("%ld vertices, %ld indices (%ld triangles)", 
                m_Terrain->GetVertexCount(), m_Terrain->GetIndexCount(),
                m_Terrain->GetTriangleCount());

    ImGui::End();
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

static void ShowTexture(uint32_t texture_id, const glm::uvec2& texSize, 
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
