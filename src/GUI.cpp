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

        if (ImGui::DragFloat3("Position", glm::value_ptr(pos), 0.1f))
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
            static bool optionsChanged = false;
            static bool autoUpdate = false;
            static bool useFallOffMap = false;
            
            static int terrainSize = m_Terrain->GetSize().x;
            static int terrainLastSize = terrainSize;
            static float tileScale = m_Terrain->GetTileScale();
            static float heightScale = m_Terrain->GetHeightScale();
            static float edge0 = m_Terrain->GetFallOffMapEdge0();
            static float edge1 = m_Terrain->GetFallOffMapEdge1();

            // (?) Resizes the terrain along with its height map
            optionsChanged |= ImGui::SliderInt("Terrain size", &terrainSize, 4, 2048);
            // (?) Scales the size of a terrain tile
            optionsChanged |= ImGui::SliderFloat("Tile scale", &tileScale, 0.01f, 1.f);
            // (?) Scales the height values of terrain's height map
            optionsChanged |= ImGui::SliderFloat("Height scale", &heightScale, 1.f, 32.f);
            optionsChanged |= ImGui::Checkbox(" Use Falloff Map", &useFallOffMap);
            if (useFallOffMap)
            {
                optionsChanged |=
                    ImGui::SliderFloat("Edge0", &edge0, 0.f, edge1-0.01);
                optionsChanged |=
                    ImGui::SliderFloat("Edge1", &edge1, edge0+0.01, 2.f);
            }

            ImGui::NewLine();
            const bool kGeneratePressed = ImGui::Button("Generate");

            // TODO if noiseMapChanged as well
            if ( optionsChanged && (kGeneratePressed || autoUpdate) )
            {
                if (terrainSize != terrainLastSize)
                {
                    m_NoiseMap->SetSize(glm::uvec2(terrainSize, terrainSize));
                    m_NoiseMap->GenerateValues();
                    m_Terrain->SetSize(glm::uvec2(terrainSize, terrainSize));

                    terrainLastSize = terrainSize;
                }

                m_Terrain->SetTileScale(tileScale);
                m_Terrain->SetHeightScale(heightScale);
                m_Terrain->UseFallOffMap(useFallOffMap);
                m_Terrain->SetFallOffMapEdge0(edge0);
                m_Terrain->SetFallOffMapEdge1(edge1);
                m_Terrain->Generate();

                optionsChanged = false;
                m_TerrainChanged = true;
            }
            ImGui::SameLine();
            ImGui::Checkbox("Auto", &autoUpdate);
        }
        ImGui::Separator();

        if (ImGui::TreeNodeEx("Noise Map Generation", ImGuiTreeNodeFlags_DefaultOpen))
        {
            static bool optionsChanged = false;
            static float scale = m_NoiseMap->GetScale();
            static int32_t seed = m_NoiseMap->GetSeed();
            static float offset = m_NoiseMap->GetOffset();
            static int octaves = m_NoiseMap->GetOctaves();
            static float gain = m_NoiseMap->GetGain();
            static float lacunarity = m_NoiseMap->GetLacunarity();

            optionsChanged |= ImGui::DragInt("Seed", &seed);
            optionsChanged |= ImGui::DragFloat("Scale", &scale, 0.1f, 0.001f);
            optionsChanged |= ImGui::DragFloat("Offset", &offset, 1.0f, -10000.f, 10000.f);
            // TODO (?)
            optionsChanged |= ImGui::SliderInt("Octaves", &octaves, 1, 32);
            optionsChanged |= ImGui::DragFloat("Gain (Persistence)", &gain, 0.01f, 0.f, 1.f);
            optionsChanged |= ImGui::DragFloat("Lacunarity", &lacunarity, 0.01f, 1.f, 100.f);

            ShowTexture(m_NoiseMap->GetTexture()->GetID(), m_NoiseMap->GetSize(), 128, 128);

            ImGui::NewLine();

            static bool autoUpdate = false;
            static bool autoUpdateTerrain = false;

            bool kUpdatePressed = ImGui::Button("Update");
            ImGui::SameLine();
            ImGui::Checkbox("Auto", &autoUpdate);
            ImGui::SameLine();
            bool kUpdateTerrainPressed = ImGui::Button("Update Terrain");
            ImGui::SameLine();
            ImGui::Checkbox("Auto##1", &autoUpdateTerrain);

            if ( optionsChanged && ( kUpdatePressed || autoUpdate ) )
            {
                m_NoiseMap->SetSeed(seed);
                m_NoiseMap->SetOctaves(octaves);
                m_NoiseMap->SetScale(scale);
                m_NoiseMap->SetOffset(offset);
                m_NoiseMap->SetGain(gain);
                m_NoiseMap->SetLacunarity(lacunarity);

                m_NoiseMap->GenerateValues();
                m_NoiseMap->UpdateTexture();

                optionsChanged = false;
                m_NoiseMapChanged = true;
            }

            if ( m_NoiseMapChanged && ( kUpdateTerrainPressed || autoUpdateTerrain ) )
            {
                m_Terrain->Generate();
                m_NoiseMapChanged = false;
                m_TerrainChanged = true;
            }

            ImGui::TreePop();
        }
        ImGui::Separator();
        
        ImGui::Separator();
        if (ImGui::TreeNodeEx("Regions"))
        {
            // TODO global settings
            ImGui::Text("Global settings");

            static float globalTintStrength = m_Regions.size() > 0 ? m_Regions[0].tintStrength : 0.0;
            static float globalBlendStrength = m_Regions.size() > 0 ? m_Regions[0].blendStrength : 0.0;
            static float globalTextureScale = m_Regions.size() > 0 ? m_Regions[0].scale : 1.0;

            ImGui::PushItemWidth(200);
            bool globalOptionsChanged =
                ImGui::DragFloat("Tint Strength", &globalTintStrength, 0.01f, 0.0f, 1.0f);
            globalOptionsChanged |=
                ImGui::DragFloat("Blend range", &globalBlendStrength, 0.01f, 0.0f, 1.0f);
            globalOptionsChanged |=
                ImGui::DragFloat("Texture Scale", &globalTextureScale, 0.1f, 0.0f, 100.0f);

            static const std::string kStrMaxRegionCount =
                std::to_string(s_kMaxRegionCount);
            ImGui::LabelText(kStrMaxRegionCount.c_str(), "Maximum number of regions:");
            ImGui::PopItemWidth();

            ImGui::Separator();

            auto& changed = m_TerrainChanged;
            for (uint32_t i = 0; i < m_Regions.size(); ++i)
            {
                auto& region = m_Regions[i];
                if (globalOptionsChanged)
                {
                    region.tintStrength = globalTintStrength;
                    region.blendStrength = globalBlendStrength;
                    region.scale = globalTextureScale;
                    changed = true;
                }

                bool AddRegionOptionsShown = false;
                ImGui::PushID(i);
                ImGui::SetNextItemOpen(true, ImGuiCond_Once);
                if ( ImGui::TreeNode(region.name.c_str()) )
                {
                    ImGui::SameLine();
                    if (m_Regions.size() < s_kMaxRegionCount && ImGui::Button("+"))
                    {
                        // Insert region
                        if (m_Regions.size() < s_kMaxRegionCount)
                        {
                            m_Regions.insert(m_Regions.begin() + i, Region(m_Regions.size()));
                            m_TerrainChanged = true;
                        }
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("-"))
                    {
                        m_Regions.erase(m_Regions.begin() + i);
                        m_TerrainChanged = true;
                    }
                    AddRegionOptionsShown = true;

                    ImGui::PushItemWidth(256);

                    changed |= ImGui::DragFloat("Start Height", &region.startHeight, 0.01f, 0.0f, 1.0f);
                    changed |= ImGui::ColorEdit3("Tint", glm::value_ptr(region.tint));
                    changed |= ImGui::Combo("Texture", &region.texIndex,
                                 s_kTerrainTexturePaths.data(),
                                 s_kTerrainTexturePaths.size());
                    changed |= ImGui::DragFloat("Tint Strength", &region.tintStrength, 0.01f, 0.0f, 1.0f);
                    changed |= ImGui::DragFloat("Blend range", &region.blendStrength, 0.01f, 0.0f, 1.0f);
                    changed |= ImGui::DragFloat("Texture Scale", &region.scale, 0.1f, 0.0f, 100.0f);

                    ImGui::PopItemWidth();
                    ImGui::TreePop();
                }
                if (!AddRegionOptionsShown)
                {
                    ImGui::SameLine();
                    if (m_Regions.size() < s_kMaxRegionCount && ImGui::Button("+"))
                    {
                        // Insert region
                        if (m_Regions.size() < s_kMaxRegionCount)
                        {
                            m_Regions.insert(m_Regions.begin() + i, Region(m_Regions.size()));
                            m_TerrainChanged = true;
                        }
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("-"))
                    {
                        m_Regions.erase(m_Regions.begin() + i);
                            m_TerrainChanged = true;
                    }
                }

                ImGui::PopID();
            }

            static bool addRegionState = false;
            static char inputRegionName[65];

            if ( m_Regions.size() < s_kMaxRegionCount )
            {
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
                        m_Regions.emplace_back(inputRegionName);
                            m_TerrainChanged = true;
                        inputRegionName[0] = '\0';
                        addRegionState = false;
                    }
                }
            }

            ImGui::Separator();
            ImGui::TreePop();
        }

        if (ImGui::TreeNodeEx("Lighting" ))
        {
            auto& data = m_LightingUBOData;

            m_LightingOptionsChanged |= 
                ImGui::DragFloat3("Sun Direction", glm::value_ptr(data.sunDir), 0.1f);
            m_LightingOptionsChanged |= 
                ImGui::DragFloat("Sun Intensity", &data.sunIntensity, 0.01f, 0.0f, 5.0f);
            m_LightingOptionsChanged |=
                ImGui::ColorEdit3("Sun Color", glm::value_ptr(data.sunColor),
                                  ImGuiColorEditFlags_Float);
            m_LightingOptionsChanged |=
                ImGui::ColorEdit3("Sky Color", glm::value_ptr(data.skyColor),
                                  ImGuiColorEditFlags_Float);
            m_LightingOptionsChanged |=
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
    ImGui::Text("%u vertices, %u indices (%u triangles)", 
                m_Terrain->GetVertexCount(), m_Terrain->GetIndexCount(),
                m_Terrain->GetTriangleCount());

    ImGui::Text("Profiling data");
    if ( ImGui::BeginTable("Profiling data", 2,
                            ImGuiTableFlags_Resizable |
                            ImGuiTableFlags_BordersOuter |
                            ImGuiTableFlags_BordersV) )
    {
        const auto& kRecords = sgl::Profile::GetRecordsFromLatest();
        for (const auto& record : kRecords)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%.3f ms", record.duration);
            ImGui::TableNextColumn();
            ImGui::Text("%s::%s", record.fileName, record.name);
        }
        ImGui::EndTable();
    }

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
