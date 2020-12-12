#pragma once

#include "opengl/shader.hpp"
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

    /** @brief 1st shader added is for single texturing
     *         2nd is for multi texturing
     */
    void addShader(const std::shared_ptr<Shader>& sh);

    //~Terrain();

    //@param projView Projection view matrix 
    void render(const glm::mat4& projView) const;

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

    void onRegionsChanged();

    // For surface texture
    void setFilteringPoint() { m_surface.set_filtering(GL_NEAREST, GL_NEAREST); }

    void setFilteringLinear() { m_surface.set_linear_filtering(); }

    void enableFalloffMap(bool f)
    {
        if (f == m_useFalloff)
            return;
        m_falloffChanged = true;
        m_useFalloff = f;
    }

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

#ifndef TRIANGLE_STRIP
    uint32_t totalTriangles() const { return totalIndices() / 3; }
#else
    uint32_t totalTriangles() const { return (m_size.x * 2 -2) * (m_size.y-1); }
#endif

    // Height-based texturing
    struct Region
    {
        float toHeight;
        // TODO union Texture2D
        glm::vec4 color;
        std::string name;

        Region(const char* name, float height, const glm::vec3& c)
          : toHeight(height), color(c, 1.0), name(name) {}

        void addRegion() {}
    };

    std::vector<Region>& regions() { return m_regions; }


private:    
    void init();

    void generate();

    void generateIndices();
    void generateNormals();
    void generateBuffers();

    // For updating new data, of the same size
    void updateVerticesBuffer();
    void updateNormalsBuffer();
    void updateTextures();

    void regenerate(const glm::uvec2& newSize);

    void onTileScaleChanged(float newScale);

    void onHeightScaleChanged(float newScale);

    void onNoiseChanged();

    void initRegions();
    
    void regionColorIn(float height, uint32_t index);

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
    std::vector<uint32_t> m_indices;      ///< Indices of vertices

    // Buffers
    VertexArray m_vao;
    std::shared_ptr<VertexBuffer> m_vboVertices;
    std::shared_ptr<VertexBuffer> m_vboNormals;
    std::shared_ptr<VertexBuffer> m_vboTexels;

    // Shader, shared_ptr for probable future extensibility
    std::shared_ptr<Shader> shSingle;       ///< Single texturing
    std::shared_ptr<Shader> shMulti;        ///< Multi texturing

    ProceduralTex2D m_heightMap;          ///< Procedurally generated height map

    //------------------------------------------------------------
    // Height-based texturing - colored / textured regions based on height
    bool m_blending = true;
    
    std::vector<Region> m_regions;

    struct ImageInfo                      ///< For Surface textures
    {
        const void* data;                 ///< Expects RGB values with opacity Channel
        uint32_t width;
        uint32_t height;
    };
    std::vector<ImageInfo> m_images;      ///< For Multitexturing
    Texture2D m_surface;                  ///< Final surface texture

    // Arrays of 2D textures
    uint32_t m_colorTextures;             ///< Solid colors as textures
    std::vector<std::vector<glm::vec4>> m_colors;

    //uint32_t m_textures;
    //uint32_t m_opacityMaps;

    void initColorTextureArray();
    void fillColorTextureArray();
    void subColorAt(uint32_t i);

    glm::uvec2 maxSizeFromImages();

    //------------------------------------------------------------
    // Falloff map
    bool m_useFalloff;
    bool m_falloffChanged = false;
    std::vector<float> m_falloffMap;

    void generateFalloffMap();

    // Lague, Sebastian. Procedural Landmass Generation (E11: falloff map).
    // [online]. Available at: https://www.youtube.com/watch?v=COmtTyLCd6I
    const float fade_a = 3.f;
    const float fade_b = 2.2f;
    constexpr float fade(float v) const
    {
        return pow(v,fade_a) / (pow(v,fade_a) + pow((fade_b - fade_b * v), fade_a));
    }


};


