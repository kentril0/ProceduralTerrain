#pragma once

#include "opengl/texture2d.hpp"
#include "opengl/vertex_array.hpp"
#include "opengl/proc_texture2d.hpp"


/**
 * @brief Interface responsible for generating the terrain mesh, texturing, TODO more
 */
class Terrain
{
public:
    /**
     * @brief Sets up buffers and textures for subsequent
     *        rendering. Terrain will be set in (0,0,0) origin
     * @param x Size in x dimension
     * @param z Size in z dimension
     * @param tileScale Scaling factor of X and Z coord.
     *                   (one tile = one texture repetition).
     * @param heightScale Scaling factor of Y coord. - the height.
     * @param model Model matrix of the terrain
     */
    Terrain(uint32_t x, uint32_t z,
            float tileScale = 1.0f, float heightScale = 1.0f,
            const glm::mat4& model = glm::mat4(1.0f));

    Terrain(const glm::uvec2& size,
            float tileScale = 1.0f, float heightScale = 1.0f,
            const glm::mat4& model = glm::mat4(1.0f));

    //~Terrain();

    void render() const;

    /**
     * @brief Gets the height of the terrain at a particular point in the world space.
     *        Use bi-linear interpolation over the vertices of the triangle to find 
     *          the exact height of the point we are currently over.
     * @param position World space position to get the height from.
     * @return Height of the terrain at a particular point or if the position is not
     *         over the terrain, a very large negative number (-FLT_MAX)
     */
    float heightAt(const glm::vec3& position) const;

    // ------------------------------------------------------------------------
    // Setters
    // ------------------------------------------------------------------------
    void setModel(const glm::mat4& m) { m_model = m; m_invModel = glm::inverse(m); }

    /**@brief Sets a new size of the terrain and refills the buffers */
    void setSize(const glm::uvec2& size) { regenerate(size); }

    void setSize(uint32_t x, uint32_t z) {  regenerate(glm::uvec2(x, z)); }

    void setSize(uint32_t size) { regenerate(glm::uvec2(size, size)); }

    void setTileScale(float scale) { onTileScaleChanged(scale); }

    void setHeightScale(float scale) { onHeightScaleChanged(scale); }

    void applyNoiseMap() { onNoiseChanged(); }

    // TODO unused
    void setTexture(
        unsigned stage = 0
    );

    // ------------------------------------------------------------------------
    // Getters
    // ------------------------------------------------------------------------
    const glm::mat4& model() const { return m_model; }

    const glm::uvec2 size() const { return m_size; }

    float tileScale() const { return m_tileScale; }

    float heightScale() const { return m_heightScale; }
    
    ProceduralTex2D& heightMap() { return m_heightMap; }
    
    uint32_t totalVertices() const { return m_vertices.size(); }

    uint32_t totalIndices() const { return m_indices.size(); }

    // Height-based texturing
    struct Region
    {
        float toHeight;
        // TODO union Texture2D
        glm::vec3 color;
        std::string name;

        Region(const char* name, float height, const glm::vec3& c)
          : toHeight(height), color(c), name(name) {}

        void addRegion() {}
    };

    std::vector<Region>& regions() { return m_regions; }


private:    
    void generate();

    void generateIndices();
    void generateNormals();
    void generateBuffers();

    // For updating new data, of the same size
    void updateVerticesBuffer();
    void updateNormalsBuffer();

    void regenerate(const glm::uvec2& newSize);

    void onTileScaleChanged(float newScale);

    void onHeightScaleChanged(float newScale);

    void onNoiseChanged();

    void initRegions();
    
    glm::vec3 regionColorIn(float height) const;

    //void renderNormals();      ///< For debugging purposes

private:
    glm::uvec2 m_size;

    float m_tileScale;             ///< Vertex's X and Z coords multiplier
    float m_heightScale;           ///< Vertex's Y coord multiplier

    glm::vec2 m_worldSize;         ///< Scaled (World) terrain size
    glm::vec2 m_worldHalfSize;     ///< Centered to (0,0,0)

    glm::mat4 m_model;             ///< Model matrix
    glm::mat4 m_invModel;          ///< Inverse model matrix

    // Data
    std::vector<glm::vec3> m_vertices;
    std::vector<glm::vec3> m_normals;
    std::vector<glm::vec2> m_texCoords;
    std::vector<glm::vec3> m_colors;
    std::vector<uint32_t> m_indices;      ///< Indices of vertices

    // Buffers
    VertexArray m_vao;
    std::shared_ptr<VertexBuffer> m_vboVertices;
    std::shared_ptr<VertexBuffer> m_vboNormals;
    std::shared_ptr<VertexBuffer> m_vboTexels;
    std::shared_ptr<VertexBuffer> m_vboColors;

    ProceduralTex2D m_heightMap;          ///< Procedurally generated height map

    // Height-based texturing - colored / textured regions based on height
    std::vector<Region> m_regions;


    std::vector<std::shared_ptr<Texture2D>> m_textures;    ///< Surface textures
    Texture2D m_surface;                  ///< Final surface texture
};


