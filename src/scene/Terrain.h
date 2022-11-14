/**
 *  Copyright (c) 2022 ProceduralTerrain authors Distributed under MIT License 
 * (http://opensource.org/licenses/MIT)
 */

#pragma once

#include <array>
#include <vector>
#include <map>
#include <memory>

#include <glm/glm.hpp>

#include <SGL/opengl/VertexArray.h>


/**
 * @brief Interface responsible for generating the terrain mesh, texturing, TODO more
 */
class Terrain
{
public:
    static std::unique_ptr<Terrain> CreateUniq(
        const glm::uvec2& size,
        const std::vector<float>& heightMap);
public:
    /**
     * @brief Generates vertices for rendering
     * @param size Size in the X and Z axis, centered at the origin
     */
    Terrain(const glm::uvec2& size,
            const std::vector<float>& heightMap);

    ~Terrain();

    void Render() const;

    /**
     * @brief Recreates vertex data based on set values of data members
     *   and heightMap values */
    // TODO call it "Update?"
    void Generate();

    void UseFallOffMap(bool enabled) { m_UseFallOffMap = enabled; }

    // Getters
    glm::uvec2 GetSize() const { return m_Size; }
    float GetTileScale() const { return m_TileScale; }
    float GetHeightScale() const { return m_HeightScale; }

    uint32_t GetVertexCount() const { return m_Size.x * m_Size.y; }
    uint32_t GetIndexCount() const { return m_Indices.size(); }
    uint32_t GetTriangleCount() const { return m_Indices.size() / 3; }

    float GetFallOffMapEdge0() const { return m_FallOffEdge0; }
    float GetFallOffMapEdge1() const { return m_FallOffEdge1; }

    // Setters
    void SetSize(const glm::vec2& size) { m_Size = size; }
    void SetTileScale(float scale) { m_TileScale = scale; }
    void SetHeightScale(float scale) { m_HeightScale = scale; }

    void SetFallOffMapEdge0(float edge0) { m_FallOffEdge0 = edge0; }
    void SetFallOffMapEdge1(float edge1) { m_FallOffEdge1 = edge1; }

private:
    using Position = glm::vec3;
    using Normal = glm::vec3;
    using TexCoord = glm::vec2;
    using Index = uint32_t;

    // Interleaved Data, might be better for the GPU
    struct Vertex {
        Position position;
        Normal   normal;
        TexCoord texCoord;

        Vertex() : position(0), normal(0), texCoord(0) {}
    };

private:
    // @return TODO should be in range [0,1]
    inline float GetHeight(size_t index) const
    {
        return m_HeightMap[ glm::min(index, m_HeightMap.size()-1) ];
    }

    inline float GetHeightScaled(size_t index) const
    {
        return GetHeight(index) * m_HeightScale;
    }

    // Positions in world units are [0, ..., size-1] 
    glm::vec2 GetWorldSize() const;

    // Generates values from top-left (0,0) to bottom right (1,1)
    void GenerateTexCoords();
    void GeneratePositions();
    void GenerateNormals();
    void GenerateIndices();

    void UpdateVAO();

    void SetupColorRegions();
    void FillColorRegionSearchMap();
    void GenerateColorData();

    void GenerateFallOffMap();
    void ApplyFallOffMap();

    constexpr float Smoothstep(float edge0, float edge1, float x) const {
        float t = glm::clamp((x - edge0) / (edge1 - edge0), 0.f, 1.f);
        return t * t * (3.f - 2.f * t);
    }

private:
    const std::vector<float>& m_HeightMap;

    glm::uvec2 m_Size{ 0 };
    float m_TileScale{ 1.0 }; // Scaling factor of X and Z coord (per tile)
    float m_HeightScale{ 1.0 }; // Scaling factor of Y coord, the height

    // -------------------------------------------------------------------------
    // Data on CPU will be "batched"
    //  better for updating data, and in render, for e.g. collision detection

    std::vector<Position> m_Positions;
    std::vector<Normal> m_Normals;
    std::vector<TexCoord> m_TexCoords;
    std::vector<Index>  m_Indices;

    sgl::VertexArray m_VAO;

    // -------------------------------------------------------------------------
    // Falloff map

    std::vector<float> m_FallOffMap;
    bool m_UseFallOffMap{ false };

    float m_FallOffEdge0{ 0.0 };
    float m_FallOffEdge1{ 1.0 };
};
