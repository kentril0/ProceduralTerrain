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

#include <SGL/opengl/Shader.h>
#include <SGL/opengl/VertexArray.h>


#include "ProceduralTexture2D.h"


/**
 * @brief Interface responsible for generating the terrain mesh, texturing, TODO more
 */
class Terrain
{
public:
    static constexpr glm::uvec2 s_kDefaultSize{ 32, 32 };

    // TODO proxy??
    static std::unique_ptr<Terrain> CreateUniq(
        const std::shared_ptr<sgl::Shader>& shader,
        const std::vector<float>& heightMap,
        const glm::uvec2& size = s_kDefaultSize);
public:
    /**
     * @brief TODO
     * @param shader Shader to draw the terrain
     * @param size Size in the X and Z axis, centered at the origin
     */
    Terrain(const std::shared_ptr<sgl::Shader>& shader,
            const std::vector<float>& heightMap,
            const glm::uvec2& size = s_kDefaultSize);

    ~Terrain();

    /**
     * @param projView Projection view matrix
     * @param camPosition Position of the camera
     */
    void Render(const glm::mat4& projView,
                const glm::vec3& camPosition) const;

    /**
     * @param position World space position to get the height at.
     * @return The height of the terrain at a particular point in the world.
     *  Uses bilinear interpolation over the vertices of the triangle to find
     *  the height of the point at 'position'.
     *  If 'position' is beyond terrain bounds, then the height returned
     *  is a very large negative number (-FLT_MAX).
     */
    float GetHeightAt(const glm::vec3& position) const;

    /**
     * @brief Recreates vertex data based on set values of data members
     *   and heightMap values */
    void Generate();

    void EnableFallOff(bool enable) {}

    // Getters
    glm::uvec2 GetSize() const { return m_Size; }
    float GetTileScale() const { return m_TileScale; }
    float GetHeightScale() const { return m_HeightScale; }

    size_t GetVertexCount() const { return m_Size.x * m_Size.y; }
    size_t GetIndexCount() const { return m_Indices.size(); }
    size_t GetTriangleCount() const { return 0; }

    // Setters
    void SetSize(const glm::vec2& size) { m_Size = size; }
    void SetTileScale(float scale) { m_TileScale = scale; }
    void SetHeightScale(float scale) { m_HeightScale = scale; }

private:
    using Position = glm::vec3;
    using Normal = glm::vec3;
    using TexCoord = glm::vec2;
    using Color = glm::vec3;
    using Index = uint32_t;

    // Interleaved Data, might be better for the GPU
    struct Vertex {
        Position position;
        Normal   normal;
        TexCoord texCoord;
        Color    color;

        Vertex() : position(0), normal(0), texCoord(0), color(0) {}
    };
private:
    inline float GetHeight(size_t index) const
    {
        return m_HeightMap[ glm::min(index, m_HeightMap.size()-1) ] *
               m_HeightScale;
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

private:
    std::shared_ptr<sgl::Shader> m_Shader;
    const std::vector<float>& m_HeightMap;

    glm::uvec2 m_Size{ 0 };
    float m_TileScale{ 1.0 }, // Scaling factor of X and Z coord (per tile)
          m_HeightScale{ 1.0 }; // Scaling factor of Y coord, the height

    // -------------------------------------------------------------------------
    // Data on CPU will be "batched"
    //  better for updating data, and in render, for e.g. collision detection

    std::vector<Position> m_Positions;
    std::vector<Normal> m_Normals;
    std::vector<TexCoord> m_TexCoords;
    std::vector<Color> m_Colors;

    std::vector<Index>  m_Indices;

    sgl::VertexArray m_VAO;

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
        ColorRegion{"Water Deep",    0.3,  glm::vec3(0.0, 0.0, 0.8)       },
        ColorRegion{"Water Shallow", 0.4,  glm::vec3(54, 103, 199)/255.f  },
        ColorRegion{"Sand",          0.45, glm::vec3(210, 208, 125)/255.f },
        ColorRegion{"Grass",         0.55, glm::vec3(86, 152, 23)/255.f   },
        ColorRegion{"Trees",         0.6,  glm::vec3(62, 107, 18)/255.f   },
        ColorRegion{"Rock",          0.7,  glm::vec3(90, 69, 60)/255.f    },
        ColorRegion{"Higher Rock",   0.9,  glm::vec3(75, 60, 53)/255.f    },
        ColorRegion{"Snow",          1.0,  glm::vec3(1.0, 1.0, 1.0)       }
    };
    static constexpr glm::vec3 s_kDefaultColor{1.0};

    // Colors per height range
    std::map<float, glm::vec3> m_ColorRegionSearchMap;

};
