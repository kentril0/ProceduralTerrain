#include "core/pch.hpp"
#include "terrain.hpp"

#include <cstring>

#define TRIANGLES_PER_QUAD 2
#define INDICES_PER_TRIANGLE 3


Terrain::Terrain(uint32_t x, uint32_t y,
                 const std::shared_ptr<Texture2D>& tex1,
                 const std::shared_ptr<Texture2D>& tex2,
                 const std::shared_ptr<Texture2D>& tex3,
                 const float tile_scale, const float height_scale,
                 const glm::mat4& model)
: size(x, y),
  tile_scale(tile_scale),
  height_scale(height_scale),
  model(model),
  inv_model(glm::inverse(model))
{
    this->tex1 = tex1;
    this->tex2 = tex2;
    this->tex3 = tex3;
    this->tex1->set_repeat();
    this->tex2->set_repeat();
    this->tex3->set_repeat();

    generate();
}

Terrain::~Terrain()
{
    // Delete buffers
    // Delete textures
}

void Terrain::generate()
{
    // ---------------------------------------------------------------- 
    // Set up vertices according to the image dimensions
    const uint32_t width = size.x;
    const uint32_t height = size.y;
    LOG_INFO("Terrain dimensions: " << width << " x " << height);

    // One vertex per pixel
    // TODO interpolation factor
    const uint32_t total_vertices = width * height;
    LOG_INFO("Terrain: Number of vertices " << total_vertices);

    vertices.resize(total_vertices);
    normals.resize(total_vertices);
    tex_coords.resize(total_vertices);
    colors.resize(total_vertices);

    // Calculate size of the terrain in world units
    // The dimensions in world units are [0, ..., size-1] 
    // TODO change
    world_size = glm::vec2((width -1) * tile_scale, 
                             (height -1) * tile_scale);
    world_half_size = glm::vec2(world_size.x * 0.5f, world_size.y * 0.5f);
    LOG_INFO("Terrain world dimensions: " << world_size.x << " x " << world_size.y);

    float S, T, X, Y, Z;
    for (uint32_t j = 0; j < height; ++j)
    {
        for (uint32_t i = 0; i < width; ++i)
        {
            uint32_t index = (j * width) + i;

            // Sample the height - value between <0; 1>
            // TODO
            float height_value = 0.1;

            // Texture coords of the terrain from top-left (0,0) to bottom right (1,1)
            S = (i / (float)(width -1));
            T = (j / (float)(height-1));

            // Compute vertex positions in world units
            // Center the terrain around the local origin to the terrain dimensions:
            //  X: [-world_half_size.x, .., world_half_size.x]
            //  Z: [-world_half_size.y, ..., world_half_size.y]
            X = (S * world_size.x) - world_half_size.x;
            Y = height_value * height_scale;
            Z = (T * world_size.y) - world_half_size.y;

            tex_coords[index] = glm::vec2(S, T);
            vertices[index] = glm::vec3(X, Y, Z);
            normals[index] = glm::vec3(0);

            colors[index] = glm::vec4(1.0f);
        }
    }

    // ---------------------------------------------------------------- 
    // Generate buffers
    generate_indices();
    generate_normals();

    generate_buffers();
    LOG_OK("Terrain has been loaded!");

    //vertices.clear();
    //normals.clear();
    //tex_coords.clear();
    //colors.clear();
    //indices.clear();
}

/**
 * @brief Builds an index buffer by constructing 2 triangles with vertices
 *        arranged in a CCW winding order (the top triangle t0 = (v0, v3, v1),
 *        the bottom triangle t1 = (v0, v2, v3)): v0 ---- v1
 *                                                |  \  t0 |
 *                                                |    \   |
 *                                                | t1   \ |
 *                                                v2 ---- v3
 */
void Terrain::generate_indices()
{
    // 2 triangles for every quad of the terrain mesh
    const uint32_t triangles = (size.x -1) * (size.y -1) * TRIANGLES_PER_QUAD;

    // 3 indices for each triangle in the terrain mesh
    indices.resize(triangles * INDICES_PER_TRIANGLE);

    uint32_t index = 0;
    for (uint32_t j = 0; j < (size.y -1); ++j)
    {
        for (uint32_t i = 0; i < (size.x -1); ++i)
        {
            uint32_t vertex_index = (j * size.x) + i;

            // Top triangle (v0, v3, v1)
            indices[index++] = vertex_index;
            indices[index++] = vertex_index + size.x + 1;
            indices[index++] = vertex_index + 1;

            // Bottom triangle (v0, v2, v3)
            indices[index++] = vertex_index;
            indices[index++] = vertex_index + size.x;
            indices[index++] = vertex_index + size.x + 1;
        }
    }
}

/**
 * @brief Generates normal of each triangle
 */
void Terrain::generate_normals()
{
    // Compute normal for each triangle as a cross product of its edges
    glm::vec3 v0, v1, v2, normal;
    for (uint32_t i = 0; i < indices.size(); i += 3)
    {
        v0 = vertices[indices[i + 0]];
        v1 = vertices[indices[i + 1]];
        v2 = vertices[indices[i + 2]];

        normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));

        // Save for each vertex
        normals[indices[i + 0]] += normal;
        normals[indices[i + 1]] += normal;
        normals[indices[i + 2]] += normal;
    }

    // Normalize each vertex normal
    const glm::vec3 up(0.0f, 1.0f, 0.0f);

    for (uint32_t i = 0; i < normals.size(); ++i)
    {
        normals[i] = glm::normalize(normals[i]);
    }
}

void Terrain::generate_buffers()
{
    // TODO to unique if possible
    std::shared_ptr vbo_vertices = std::make_shared<VertexBuffer>(
        vertices.size() * sizeof(glm::vec3), vertices.data());

    std::shared_ptr vbo_normals = std::make_shared<VertexBuffer>(
        normals.size() * sizeof(glm::vec3), normals.data());

    std::shared_ptr vbo_texs = std::make_shared<VertexBuffer>(
        tex_coords.size() * sizeof(glm::vec2), tex_coords.data());

    std::shared_ptr vbo_colors = std::make_shared<VertexBuffer>(
        colors.size() * sizeof(glm::vec4), colors.data());

    std::shared_ptr ibo = std::make_shared<IndexBuffer>(indices.size(), indices.data());

    vbo_vertices->set_layout(BufferLayout({{ElementType::Float3, "Position"}}));
    vbo_normals->set_layout(BufferLayout({{ElementType::Float3, "Normal"}}));
    vbo_texs->set_layout(BufferLayout({{ElementType::Float2, "TexCoords"}}));
    vbo_colors->set_layout(BufferLayout({{ElementType::Float4, "Color"}}));

    vao.add_vertex_buffer(vbo_vertices);
    vao.add_vertex_buffer(vbo_normals);
    vao.add_vertex_buffer(vbo_texs);
    vao.add_vertex_buffer(vbo_colors);
    vao.set_index_buffer(ibo);
}

/**
 * @brief Terrain supports up to 3 texture stages (0-2). Each texture stage
 *        will be blended with next to produce a realistic looking terrain.
 *        For each texture stage, a texture can be specified to be used by
 *        loading the texture into the texture stage using this method.
 * @param ID Texture ID of newly generated texture with already supplied image
 *                as according to the call from Resource manager
 * @param stage Texture stage to bind the texture to. Valid values are
 *                      between 0 and 2 for 3 possible texture stages.
 *                      Texture stages:
 *                          0: used for the lowest points of the terrain.
 *                          1: is blended between poinst 0 and 2 depending on
 *                             the blending method used (height based blending
 *                             or slope based blending)  
 *                          2: used for the highest points in the terrain.
 */
void Terrain::set_texture(unsigned stage)
{
    tex1->set_repeat();
        //glTextureParameteri(ID, GL_TEXTURE_WRAP_S, GL_REPEAT);
        //glTextureParameteri(ID, GL_TEXTURE_WRAP_T, GL_REPEAT);
        //glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}


/**
 * @brief Render function for the terrain
 *
 */
void Terrain::render()
{
    // Disable lighting because it changes the primary color of the vertices that are
    // used for the multitexture blending.
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //glActiveTexture(GL_TEXTURE0);
    //glActiveTexture(GL_TEXTURE1);
    //glActiveTexture(GL_TEXTURE2);
    //tex1->bind();
    //tex1->bind_unit(0);
    ///tex2->bind_unit(1);
    ///tex3->bind_unit(2);

    vao.bind();

    // Rendering using indexed element arrays
    //glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
    glDrawElements(GL_TRIANGLE_STRIP, indices.size(), GL_UNSIGNED_INT, nullptr);
}


/**
 * @brief Gets the height of the terrain at a particular point in the world space.
 *        Use bi-linear interpolation over the vertices of the triangle to find 
 *          the exact height of the point we are currently over.
 * @param position World space position to get the height from.
 * @return Height of the terrain at a particular point or if the position is not
 *         over the terrain, a very large negative number (-FLT_MAX)
 */
float Terrain::height_at(const glm::vec3& position)
{
    float height = -FLT_MAX;

    // Get the 4 vertices of the quad

    // Get the position in terrain local space
    //  The position may lay between 2 vertex indices in which case the integer part
    //  will be the index of the top-left vertex, while the fractional part will be
    //  the ratio between the top-left and top-right vertex in the X-component
    //  and similar for the value in the Z-component
    glm::vec3 terrain_pos = glm::vec3(inv_model * glm::vec4(position, 1.0f));
    glm::vec3 inv_tile_scale(1.0f / tile_scale, 0.0f, 1.0f / tile_scale);

    // Calculate an offset from the center and scale to get the vertex indices
    glm::vec3 offset(world_half_size.x, 0.0f, world_half_size.y);

    // Rational index of the vertices on the terrain mesh
    glm::vec3 vertex_indices = (terrain_pos + offset) * inv_tile_scale;

    // Compute the indices of the 4 verts that make up the quad
    int u0 = (int)floorf(vertex_indices.x);
    int u1 = u0 + 1;
    int v0 = (int)floorf(vertex_indices.z);
    int v1 = v0 + 1;

    if (u0 >= 0 && u1 < (int)size.x
        && v0 >=0 && v1 < (int)size.y)
    {
        // Top-left vertex
        glm::vec3 p00 = vertices[(v0 * size.x) + u0];
        // Top-right vertex
        glm::vec3 p10 = vertices[(v0 * size.x) + u1];
        // Bottom-left vertex
        glm::vec3 p01 = vertices[(v1 * size.x) + u0];
        // Bottom-right vertex
        glm::vec3 p11 = vertices[(v1 * size.x) + u1];

        // Find the triangle
        // If the fractional part of the vertex index in the X-axis is greater than
        //  the fractional part of the vertex index in the Z-axis -> inside top triangle
        //  otherwise inside the bottom triangle
        float percent_u = vertex_indices.x - u0;
        float percent_v = vertex_indices.z - v0;

        glm::vec3 du, dv;
        if (percent_u > percent_v)
        {
            // Top triangle
            //  compute the change in position between the vertices of the quad
            du = p10 - p00;
            dv = p11 - p10;
        }
        else
        {
            // Bottom triangle
            du = p11 - p01;
            dv = p01 - p00;
        }

        // Compute the exact position based on the position of the top-left vertex
        //  and the ratio between the vertices
        glm::vec3 height_pos = p00 + (du * percent_u) + (dv * percent_v);

        // Convert back to world-space by multiplying by the terrain's world matrix
        height_pos = glm::vec3(model * glm::vec4(height_pos, 1));
        height = height_pos.y;
    }

    return height;
}

