#pragma once

#include "opengl/texture2d.hpp"
#include "opengl/vertex_array.hpp"


/**
 * @brief Interface responsible for generating the terrain mesh, texturing, TODO more
 */
class Terrain
{
public:
    /**
     * @brief , sets up buffers and textures for subsequent
     *        rendering.
     * @param x_dim Size in x dimension
     * @param z_dim Size in z dimension
     * @param tex1 Texture TODO NOT USED
     * @param tex2 Texture TODO NOT USED
     * @param tex3 Texture TODO NOT USED
     * @param tile_scale Scaling factor of X and Z coord.
     *                   (one tile = one texture repetition).
     * @param height_scale Scaling factor of Y coord. - the height.
     * @param model Model matrix of the terrain
     */
    Terrain(uint32_t x_dim, uint32_t z_dim,
            const std::shared_ptr<Texture2D>& tex1, 
            const std::shared_ptr<Texture2D>& tex2 = nullptr, 
            const std::shared_ptr<Texture2D>& tex3 = nullptr, 
            float tile_scale = 1.0f, const float height_scale = 1.0f,
            const glm::mat4& model = glm::mat4(1.0f));

    ~Terrain();

    void render();

    /** @return Height of the terrain at world position */
    float height_at(const glm::vec3& position);

    // TODO unused
    void set_texture(
        unsigned stage = 0
    );

    void debug_render();

    void set_model(const glm::mat4& m) { model = m; inv_model = glm::inverse(model); }

    const glm::mat4& get_model() { return model; }

    const glm::uvec2 get_size() { return size; }

private:    

    void generate();

    void generate_indices();
    void generate_normals();
    void generate_buffers();

    void render_normals();      ///< For debugging purposes

private:
    glm::uvec2 size;
    glm::vec2 world_size;   ///< Scaled (World) terrain size
    glm::vec2 world_half_size;

    float tile_scale;       ///< Vertex's X and Z coords multiplier
    float height_scale;     ///< Vertex's Y coord multiplier

    glm::mat4 model;        ///< Model matrix
    glm::mat4 inv_model;    ///< Inverse model matrix

    // Buffers
    // TODO keep?
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> tex_coords;
    std::vector<glm::vec4> colors;      ///< +multitexture blending ratios
    std::vector<uint32_t> indices;      ///< Indices of vertices

    // VAO
    VertexArray vao;

    // Textures
    static const uint8_t NUM_TEXTURES = 3;     ///< Number of textures used across the terrain
    std::shared_ptr<Texture2D> tex1;
    std::shared_ptr<Texture2D> tex2;
    std::shared_ptr<Texture2D> tex3;
};

