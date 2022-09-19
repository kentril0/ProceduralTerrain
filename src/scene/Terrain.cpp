/**
 *  Copyright (c) 2022 ProceduralTerrain authors Distributed under MIT License 
 * (http://opensource.org/licenses/MIT)
 */

#include "Terrain.h"

#include <memory>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#define SGL_DEBUG
#define SGL_ENABLE_ASSERTS
#include <SGL/SGL.h>

#define TRIANGLES_PER_QUAD 2
#define INDICES_PER_TRIANGLE 3

//#define DEBUG_STRIP

std::unique_ptr<Terrain> Terrain::CreateUniq(
    const std::shared_ptr<sgl::Shader>& shader,
    const std::vector<float>& heightMap,
    const glm::uvec2& size)
{
    return std::make_unique<Terrain>(shader, heightMap, size);
}

// =============================================================================

Terrain::Terrain(const std::shared_ptr<sgl::Shader>& shader,
                 const std::vector<float>& heightMap,
                 const glm::uvec2& size)
    : m_Shader(shader),
      m_HeightMap(heightMap),
      m_Size(size)
{
    SetupColorRegions();
    Generate();
}

Terrain::~Terrain()
{

}

void Terrain::Generate()
{
    GenerateTexCoords();
    GeneratePositions();
    GenerateIndices();
    GenerateNormals();

    GenerateColorData();

    UpdateVAO();
}

void Terrain::GenerateTexCoords()
{
    m_TexCoords.resize( GetVertexCount() );

    for (uint32_t y = 0; y < m_Size.y; ++y)
        for (uint32_t x = 0; x < m_Size.x; ++x)
        {
            const uint32_t kIndex = y * m_Size.x + x;
            m_TexCoords[kIndex].x = x / static_cast<float>(m_Size.x - 1);
            m_TexCoords[kIndex].y = y / static_cast<float>(m_Size.y - 1);
        }
}

void Terrain::GeneratePositions()
{
    const glm::vec2 kWorldSize = GetWorldSize();
    const glm::vec2 kWorldHalfSize = kWorldSize*0.5f;

    m_Positions.resize( GetVertexCount() );

    // Generates values from top-left (0,0) to bottom right (1,1)
    for (uint32_t y = 0; y < m_Size.y; ++y)
        for (uint32_t x = 0; x < m_Size.x; ++x)
        {
            const uint32_t kIndex = y * m_Size.x + x;

            auto& position = m_Positions[kIndex];
            position.x = m_TexCoords[kIndex].x *
                         kWorldSize.x - kWorldHalfSize.y;
            position.y = GetHeight(kIndex);
            position.z = m_TexCoords[kIndex].y *
                         kWorldSize.y - kWorldHalfSize.y;
        }
}

void Terrain::GenerateIndices()
{
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
    m_Normals.resize( GetVertexCount(), glm::vec3(0.0f) );
    SGL_ASSERT(m_Indices.size() % 3 == 0 )

    for (uint32_t i = 0; i < m_Indices.size(); i += 3)
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
    const auto kVertexCount = GetVertexCount();
    std::vector<Vertex> vertices(kVertexCount);

    for (size_t i = 0; i < kVertexCount; ++i)
    {
        vertices[i].position = m_Positions[i];
        vertices[i].normal = m_Normals[i];
        vertices[i].texCoord = m_TexCoords[i];
        vertices[i].color = m_Colors[i];
    }

    // TODO recreate or reallocate?

    auto vbo = sgl::VertexBuffer::Create(
        vertices.data(),
        vertices.size() * sizeof(Vertex)
    );

    vbo->SetLayout({
        { sgl::ElementType::Float3, "position" },
        { sgl::ElementType::Float3, "normal" },
        { sgl::ElementType::Float2, "texCoord" },
        { sgl::ElementType::Float3, "color" }
    });

    auto ibo = sgl::IndexBuffer::Create(
        m_Indices.data(),
        m_Indices.size()
    );

    m_VAO.ClearVertexBuffers();
    m_VAO.ClearIndexBuffer();

    m_VAO.AddVertexBuffer(vbo);
    m_VAO.SetIndexBuffer(ibo);
}

void Terrain::Render(const glm::mat4& projView,
    const glm::vec3& camPosition) const
{
    m_Shader->Use();
    m_Shader->SetMat4("MVP", projView * glm::mat4(1.0));
    m_VAO.Bind();

    glDrawElements(GL_TRIANGLES,
                   m_Indices.size(),
                   GL_UNSIGNED_INT, 0);
}

void Terrain::SetupColorRegions()
{
    FillColorRegionSearchMap();
}

void Terrain::FillColorRegionSearchMap()
{
    for (const auto& region : s_kColorRegions)
    {
        m_ColorRegionSearchMap[region.heightTopBound] = region.color;
    }
}

void Terrain::GenerateColorData()
{
    m_Colors.resize( GetVertexCount() );

    for (uint32_t y = 0; y < m_Size.y; ++y)
        for (uint32_t x = 0; x < m_Size.x; ++x)
        {
            const uint32_t kIndex = y*m_Size.x + x;
            const float kHeight = GetHeight(kIndex);

            const auto colorRangeIt = 
                m_ColorRegionSearchMap.lower_bound(kHeight);
            if ( colorRangeIt != m_ColorRegionSearchMap.end() )
                m_Colors[kIndex] = colorRangeIt->second;
            else
                m_Colors[kIndex] = s_kDefaultColor;
        }
}
