/**
 *  Copyright (c) 2022 ProceduralTerrain authors Distributed under MIT License 
 * (http://opensource.org/licenses/MIT)
 */

#include "Terrain.h"

#include <memory>
#include <vector>
#include <limits>

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#define SGL_PROFILE
#include <SGL/SGL.h>

#define TRIANGLES_PER_QUAD 2
#define INDICES_PER_TRIANGLE 3


std::unique_ptr<Terrain> Terrain::CreateUniq(
    const glm::uvec2& size,
    const std::vector<float>& heightMap)
{
    return std::make_unique<Terrain>(size, heightMap);
}

// =============================================================================

Terrain::Terrain(const glm::uvec2& size,
                 const std::vector<float>& heightMap)

    : m_HeightMap(heightMap),
      m_Size(size)
{
    Generate();
}

Terrain::~Terrain()
{

}

void Terrain::Generate()
{
    SGL_PROFILE_SCOPE();

    GenerateTexCoords();
    GeneratePositions();

    if (m_UseFallOffMap)
    {
        GenerateFallOffMap();
        ApplyFallOffMap();
    }

    GenerateIndices();
    GenerateNormals();

    UpdateVAO();
}

void Terrain::GenerateTexCoords()
{
    SGL_PROFILE_SCOPE();

    const glm::uvec2 kSize = m_Size;
    m_TexCoords.resize( GetVertexCount() );

    for (uint32_t y = 0; y < kSize.y; ++y)
        for (uint32_t x = 0; x < kSize.x; ++x)
        {
            const uint32_t kIndex = y * kSize.x + x;
            m_TexCoords[kIndex].x = x / static_cast<float>(kSize.x - 1);
            m_TexCoords[kIndex].y = y / static_cast<float>(kSize.y - 1);
        }
}

void Terrain::GeneratePositions()
{
    SGL_PROFILE_SCOPE();

    const glm::vec2 kSize = m_Size;

    // Account for tile scaling
    const glm::vec2 kWorldSize = GetWorldSize();
    const glm::vec2 kCenterOffset = kWorldSize*0.5f;

    m_Positions.resize( GetVertexCount() );

    for (uint32_t y = 0; y < kSize.y; ++y)
        for (uint32_t x = 0; x < kSize.x; ++x)
        {
            const uint32_t kIndex = y * kSize.x + x;
            auto& position = m_Positions[kIndex];

            position.x = m_TexCoords[kIndex].x * kWorldSize.x - kCenterOffset.x;
            position.z = m_TexCoords[kIndex].y * kWorldSize.y - kCenterOffset.y;
            position.y = GetHeightScaled(kIndex);
        }
}

void Terrain::GenerateIndices()
{
    SGL_PROFILE_SCOPE();

    const glm::vec2 kSize = m_Size - 1U;

    m_Indices.resize(kSize.x*kSize.y *
                     TRIANGLES_PER_QUAD*INDICES_PER_TRIANGLE);

    uint32_t index = 0;
    for (uint32_t y = 0; y < kSize.y; ++y)
        for (uint32_t x = 0; x < kSize.x; ++x)
        {
            const uint32_t kVertexIndex = y * m_Size.x + x;
            // Top triangle
            m_Indices[index++] = kVertexIndex;
            m_Indices[index++] = kVertexIndex + m_Size.x + 1;
            m_Indices[index++] = kVertexIndex + 1;
            // Bottom triangle
            m_Indices[index++] = kVertexIndex;
            m_Indices[index++] = kVertexIndex + m_Size.x;
            m_Indices[index++] = kVertexIndex + m_Size.x + 1;
        }
}

void Terrain::GenerateNormals()
{
    SGL_PROFILE_SCOPE();

    m_Normals.resize( GetVertexCount(), Normal(0.0f) );

    const uint32_t kIndexCount = GetIndexCount();
    SGL_ASSERT(kIndexCount % INDICES_PER_TRIANGLE == 0 )

    for (uint32_t i = 0; i < kIndexCount; i += INDICES_PER_TRIANGLE)
    {
        const auto kIdx1 = m_Indices[i + 0];
        const auto kIdx2 = m_Indices[i + 1];
        const auto kIdx3 = m_Indices[i + 2];

        const auto& v0 = m_Positions[kIdx1];
        const auto& v1 = m_Positions[kIdx2];
        const auto& v2 = m_Positions[kIdx3];

        const Normal kNormal = glm::normalize(
            glm::cross(v1 - v0, v2 - v0)
        );

        // Save for each vertex
        m_Normals[kIdx1] += kNormal;
        m_Normals[kIdx2] += kNormal;
        m_Normals[kIdx3] += kNormal;
    }

    // Normalize each vertex normal
    for (auto& normal : m_Normals)
    {
        normal = glm::normalize(normal);
    }
}

glm::vec2 Terrain::GetWorldSize() const
{
    return glm::vec2((float)(m_Size.x-1) * m_TileScale, 
                     (float)(m_Size.y-1) * m_TileScale);
}

void Terrain::UpdateVAO()
{
    SGL_PROFILE_SCOPE();

    const auto kVertexCount = GetVertexCount();
    std::vector<Vertex> vertices(kVertexCount);

    for (size_t i = 0; i < kVertexCount; ++i)
    {
        vertices[i].position = m_Positions[i];
        vertices[i].normal = m_Normals[i];
        vertices[i].texCoord = m_TexCoords[i];
    }

    // TODO recreate or reallocate?

    auto vbo = sgl::VertexBuffer::Create(
        vertices.data(),
        vertices.size() * sizeof(Vertex)
    );

    vbo->SetLayout({
        { sgl::ElementType::Float3, "position" },
        { sgl::ElementType::Float3, "normal" },
        { sgl::ElementType::Float2, "texCoord" }
    });

    auto ibo = sgl::IndexBuffer::Create(
        m_Indices.data(),
        m_Indices.size()
    );

    m_VAO.ClearVertexBuffers();
    m_VAO.ClearIndexBuffer();

    m_VAO.AddVertexBuffer(vbo);
    m_VAO.SetIndexBuffer(ibo);

    // TODO clear generated data
}

void Terrain::Render() const
{
    m_VAO.Bind();

    glDrawElements(GL_TRIANGLES,
                   m_Indices.size(),
                   GL_UNSIGNED_INT, 0);
}

void Terrain::GenerateFallOffMap()
{
    SGL_PROFILE_SCOPE();

    m_FallOffMap.resize(m_Size.x * m_Size.y);

    for (uint32_t y = 0; y < m_Size.y; ++y)
        for (uint32_t x = 0; x < m_Size.x; ++x)
        {
            m_FallOffMap[y * m_Size.x + x] =
                Smoothstep(m_FallOffEdge0,
                           m_FallOffEdge1,
                           glm::max(
                            glm::abs(x / static_cast<float>(m_Size.x)*2.f-1.f),
                            glm::abs(y / static_cast<float>(m_Size.y)*2.f-1.f)
                           )
                );
        }
}

void Terrain::ApplyFallOffMap()
{
    SGL_PROFILE_SCOPE();

    for (uint32_t y = 0; y < m_Size.y; ++y)
        for (uint32_t x = 0; x < m_Size.x; ++x)
        {
            const uint32_t kIndex = y * m_Size.x + x;
            m_Positions[kIndex].y =
                glm::clamp(
                    m_HeightMap[kIndex] - m_FallOffMap[kIndex],
                    0.f,
                    1.f)
                * m_HeightScale;
        }
}