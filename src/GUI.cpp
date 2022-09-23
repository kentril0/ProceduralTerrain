/**
 *  Copyright (c) 2022 ProceduralTerrain authors Distributed under MIT License 
 * (http://opensource.org/licenses/MIT)
 */

#include "ProceduralTerrain.h"
#include <glm/gtc/type_ptr.hpp>


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
static void showTexture(uint32_t texture_id, const glm::uvec2& texSize, 
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

        if (ImGui::InputFloat3("Position", glm::value_ptr(pos)))
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
            static int dim = m_Terrain->GetSize().x;
            static bool autoUpdate = false;
            static bool falloff = true;
            static float tileScale = m_Terrain->GetTileScale();
            static float heightScale = m_Terrain->GetHeightScale();

            // TODO ? with  [vertices^2]
            ImGui::SliderInt("Terrain size", &dim, 4, 512);
            ImGui::SliderFloat("Tile scale", &tileScale, 0.01f, 10.f);
            ImGui::SliderFloat("Height scale", &heightScale, 0.01f, 10.f);

            ImGui::NewLine();
            if (ImGui::Button("Generate"))
            {
                m_NoiseMap->SetSize(glm::uvec2(dim, dim));
                m_NoiseMap->GenerateValues();

                m_Terrain->SetSize(glm::uvec2(dim, dim));
                m_Terrain->SetTileScale(tileScale);
                m_Terrain->SetHeightScale(heightScale);
                m_Terrain->Generate();
            }
        }
        ImGui::Separator();

        if (ImGui::TreeNodeEx("Noise Map Generation", ImGuiTreeNodeFlags_DefaultOpen))
        {
            static auto scale = m_NoiseMap->GetScale();

            if ( ImGui::DragFloat2("Scale", glm::value_ptr(scale), 
                0.01f, 0.f, 100.f) )
            {
                m_NoiseMap->SetScale(scale);
            }

            ImGui::TreePop();
        }
        ImGui::Separator();
        
        ImGui::Separator();
        if (ImGui::TreeNode("Shading"))
        {
            
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

static void showTexture(uint32_t texture_id, const glm::uvec2& texSize, 
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