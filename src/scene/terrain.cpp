#include "core/pch.hpp"
#include "terrain.hpp"

#include <cstring>

#define TRIANGLES_PER_QUAD 2
#define INDICES_PER_TRIANGLE 3


Terrain::Terrain(uint32_t x, uint32_t y,
                 float tileScale, float heightScale,
                 const glm::mat4& model)
: m_size(x, y),
  m_tileScale(tileScale),
  m_heightScale(heightScale),
  m_model(model),
  m_invModel(glm::inverse(model)),
  m_heightMap(x),
  m_surface(true)
{
    m_surface.set_internal_format(GL_RGB8);
    m_surface.set_image_format(GL_RGB);
    m_surface.set_clamp_to_edge();
    m_surface.set_filtering(GL_NEAREST, GL_NEAREST);

    initRegions();
    generate();
}

Terrain::Terrain(const glm::uvec2& size,
                 float tileScale, float heightScale,
                 const glm::mat4& model)
: m_size(size.x, size.y),
  m_tileScale(tileScale),
  m_heightScale(heightScale),
  m_model(model),
  m_invModel(glm::inverse(model)),
  m_heightMap(size.x),
  m_surface(true)
{
    m_surface.set_internal_format(GL_RGB8);
    m_surface.set_image_format(GL_RGB);
    m_surface.set_clamp_to_edge();
    m_surface.set_filtering(GL_NEAREST, GL_NEAREST);

    initRegions();
    generate();
}

// TODO function
void Terrain::generate()
{
    // Set up vertices
    const uint32_t width = m_size.x;
    const uint32_t height = m_size.y;
    LOG_INFO("Terrain dimensions: " << width << " x " << height);

    // One vertex per pixel
    // TODO interpolation factor
    const uint32_t totalVertices = width * height;
    LOG_INFO("Terrain: Number of vertices " << totalVertices);

    m_vertices.resize(totalVertices);
    m_normals.resize(totalVertices);
    m_texCoords.resize(totalVertices);
    m_colors.resize(totalVertices);

    // Calculate size of the terrain in world units
    // The dimensions in world units are [0, ..., size-1] 
    // TODO change
    m_worldSize = glm::vec2((width  -1) * m_tileScale, 
                            (height -1) * m_tileScale);

    m_worldHalfSize = glm::vec2(m_worldSize.x * 0.5f, m_worldSize.y * 0.5f);
    LOG_INFO("Terrain world dimensions: " << m_worldSize.x << " x " << m_worldSize.y);

    float S, T, X, Y, Z;
    for (uint32_t j = 0; j < height; ++j)
        for (uint32_t i = 0; i < width; ++i)
        {
            uint32_t index = (j * width) + i;

            // Sample the height - value between <0; 1>
            float heightValue = m_heightMap[index];

            // Texture coords of the terrain from top-left (0,0) to bottom right (1,1)
            S = (i / (float)(width -1));
            T = (j / (float)(height-1));

            // Compute vertex positions in world units
            // Center the terrain around the local origin to the terrain dimensions:
            //  X: [-m_worldHalfSize.x, .., m_worldHalfSize.x]
            //  Z: [-m_worldHalfSize.y, ..., m_worldHalfSize.y]
            
            X = (S * m_worldSize.x) - m_worldHalfSize.x;
            Y = heightValue * m_heightScale;
            Z = (T * m_worldSize.y) - m_worldHalfSize.y;

            m_texCoords[index] = glm::vec2(S, T);
            m_vertices[index] = glm::vec3(X, Y, Z);

            // TODO height-based texturing
            m_colors[index] = regionColorIn(heightValue);
        }

    // ---------------------------------------------------------------- 
    // Generate buffers
    generateIndices();
    generateNormals();

    generateBuffers();
    m_surface.upload((float*)&m_colors[0], m_size.x, m_size.y);

    LOG_OK("Terrain has been loaded!");

    // TODO
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
void Terrain::generateIndices()
{
    // 2 triangles for every quad of the terrain mesh
    const uint32_t triangles = (m_size.x -1) * (m_size.y -1) * TRIANGLES_PER_QUAD;

    // 3 indices for each triangle in the terrain mesh
    m_indices.resize(triangles * INDICES_PER_TRIANGLE);

    uint32_t index = 0;
    for (uint32_t j = 0; j < (m_size.y -1); ++j)
    {
        for (uint32_t i = 0; i < (m_size.x -1); ++i)
        {
            uint32_t vertexIndex = (j * m_size.x) + i;

            // Top triangle (v0, v3, v1)
            m_indices[index++] = vertexIndex;
            m_indices[index++] = vertexIndex + m_size.x + 1;
            m_indices[index++] = vertexIndex + 1;

            // Bottom triangle (v0, v2, v3)
            m_indices[index++] = vertexIndex;
            m_indices[index++] = vertexIndex + m_size.x;
            m_indices[index++] = vertexIndex + m_size.x + 1;
        }
    }
}

/**
 * @brief Generates normal of each triangle
 */
void Terrain::generateNormals()
{
    // Stored continuously
    std::fill(m_normals.begin(), m_normals.end(), glm::vec3(0));

    // Compute normal for each triangle as a cross product of its edges
    glm::vec3 normal;
    for (uint32_t i = 0; i < m_indices.size(); i += 3)
    {
        const glm::vec3& v0 = m_vertices[m_indices[i + 0]];
        const glm::vec3& v1 = m_vertices[m_indices[i + 1]];
        const glm::vec3& v2 = m_vertices[m_indices[i + 2]];

        normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));

        // Save for each vertex
        m_normals[m_indices[i + 0]] += normal;
        m_normals[m_indices[i + 1]] += normal;
        m_normals[m_indices[i + 2]] += normal;
    }

    // TODO slope
    //const glm::vec3 up(0.0f, 1.0f, 0.0f);

    // Normalize each vertex normal
    for (glm::vec3& n : m_normals)
        n = glm::normalize(n);
}

void Terrain::generateBuffers()
{
    // TODO to unique if possible
    m_vboVertices = std::make_shared<VertexBuffer>(
        m_vertices.size() * sizeof(glm::vec3), m_vertices.data());

    m_vboNormals = std::make_shared<VertexBuffer>(
        m_normals.size() * sizeof(glm::vec3), m_normals.data());

    m_vboTexels = std::make_shared<VertexBuffer>(
        m_texCoords.size() * sizeof(glm::vec2), m_texCoords.data());

    //m_vboColors = std::make_shared<VertexBuffer>(
    //    m_colors.size() * sizeof(glm::vec3), m_colors.data());

    std::shared_ptr ibo = std::make_shared<IndexBuffer>(m_indices.size(), m_indices.data());

    m_vboVertices->set_layout(BufferLayout({{ElementType::Float3, "Position"}}));
    m_vboNormals->set_layout(BufferLayout({{ElementType::Float3, "Normal"}}));
    m_vboTexels->set_layout(BufferLayout({{ElementType::Float2, "TexCoords"}}));
    //m_vboColors->set_layout(BufferLayout({{ElementType::Float3, "Color"}}));

    m_vao.add_vertex_buffer(m_vboVertices);
    m_vao.add_vertex_buffer(m_vboNormals);
    m_vao.add_vertex_buffer(m_vboTexels);
    //m_vao.add_vertex_buffer(m_vboColors);
    m_vao.set_index_buffer(ibo);
}

void Terrain::updateVerticesBuffer()
{
    m_vboVertices->set_data(m_vertices.size() * sizeof(glm::vec3), m_vertices.data());
}

void Terrain::updateNormalsBuffer()
{
    m_vboNormals->set_data(m_normals.size() * sizeof(glm::vec3), m_normals.data());
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
void Terrain::setTexture(unsigned stage)
{
    //tex1->set_repeat();
        //glTextureParameteri(ID, GL_TEXTURE_WRAP_S, GL_REPEAT);
        //glTextureParameteri(ID, GL_TEXTURE_WRAP_T, GL_REPEAT);
        //glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}


void Terrain::render() const
{
    // Disable lighting because it changes the primary color of the vertices that are
    // used for the multitexture blending.
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glActiveTexture(GL_TEXTURE0);
    //glActiveTexture(GL_TEXTURE1);
    //glActiveTexture(GL_TEXTURE2);
    //tex1->bind();
    //tex1->bind_unit(0);
    ///tex2->bind_unit(1);
    ///tex3->bind_unit(2);

    m_surface.bind();
    m_vao.bind();

    // Rendering using indexed element arrays
    //glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
    glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, nullptr);
}


/**
 * @brief Gets the height of the terrain at a particular point in the world space.
 *        Use bi-linear interpolation over the vertices of the triangle to find 
 *          the exact height of the point we are currently over.
 * @param position World space position to get the height from.
 * @return Height of the terrain at a particular point or if the position is not
 *         over the terrain, a very large negative number (-FLT_MAX)
 */
float Terrain::heightAt(const glm::vec3& position) const
{
    float height = -FLT_MAX;

    // Get the 4 vertices of the quad

    // Get the position in terrain local space
    //  The position may lay between 2 vertex indices in which case the integer part
    //  will be the index of the top-left vertex, while the fractional part will be
    //  the ratio between the top-left and top-right vertex in the X-component
    //  and similar for the value in the Z-component
    glm::vec3 terrainPos = glm::vec3(m_invModel * glm::vec4(position, 1.0f));
    glm::vec3 invTileScale(1.0f / m_tileScale, 0.0f, 1.0f / m_tileScale);

    // Calculate an offset from the center and scale to get the vertex indices
    glm::vec3 offset(m_worldHalfSize.x, 0.0f, m_worldHalfSize.y);

    // Rational index of the vertices on the terrain mesh
    glm::vec3 vertexIndices = (terrainPos + offset) * invTileScale;

    // Compute the indices of the 4 verts that make up the quad
    int u0 = (int)floorf(vertexIndices.x);
    int u1 = u0 + 1;
    int v0 = (int)floorf(vertexIndices.z);
    int v1 = v0 + 1;

    if (u0 >= 0 && u1 < (int)m_size.x
        && v0 >=0 && v1 < (int)m_size.y)
    {
        // Top-left vertex
        glm::vec3 p00 = m_vertices[(v0 * m_size.x) + u0];
        // Top-right vertex
        glm::vec3 p10 = m_vertices[(v0 * m_size.x) + u1];
        // Bottom-left vertex
        glm::vec3 p01 = m_vertices[(v1 * m_size.x) + u0];
        // Bottom-right vertex
        glm::vec3 p11 = m_vertices[(v1 * m_size.x) + u1];

        // Find the triangle
        // If the fractional part of the vertex index in the X-axis is greater than
        //  the fractional part of the vertex index in the Z-axis -> inside top triangle
        //  otherwise inside the bottom triangle
        float percentU = vertexIndices.x - u0;
        float percentV = vertexIndices.z - v0;

        glm::vec3 du, dv;
        if (percentU > percentV)
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
        glm::vec3 heightPos = p00 + (du * percentU) + (dv * percentV);

        // Convert back to world-space by multiplying by the terrain's world matrix
        heightPos = glm::vec3(m_model * glm::vec4(heightPos, 1));
        height = heightPos.y;
    }

    return height;
}

void Terrain::regenerate(const glm::uvec2& newSize)
{
    if (newSize == m_size)
        return;

    m_size = newSize;

    // TODO better
    m_vao.unbind();
    m_vao.clear();
    //m_vertices.clear();
    //m_normals.clear();
    //m_texCoords.clear();
    //m_colors.clear();
    //m_indices.clear();
    m_heightMap.setSize(m_size.x);
    generate();
}

void Terrain::onTileScaleChanged(float newScale)
{
    newScale = std::max(newScale, 0.01f);
    m_tileScale = newScale;
    const uint32_t width = m_size.x;
    const uint32_t height = m_size.y;
    m_worldSize = glm::vec2((width  -1) * m_tileScale, 
                            (height -1) * m_tileScale);
    m_worldHalfSize = glm::vec2(m_worldSize.x * 0.5f, m_worldSize.y * 0.5f);

    float S, T;
    for (uint32_t j = 0; j < height; ++j)
    {
        for (uint32_t i = 0; i < width; ++i)
        {
            uint32_t index = (j * width) + i;

            // Texture coords of the terrain from top-left (0,0) to bottom right (1,1)
            S = (i / (float)(width -1));
            T = (j / (float)(height-1));

            // Compute vertex positions in world units
            m_vertices[index].x = (S * m_worldSize.x) - m_worldHalfSize.x;
            m_vertices[index].z = (T * m_worldSize.y) - m_worldHalfSize.y;
        }
    }

    // Update buffers
    generateNormals();

    updateVerticesBuffer();
    updateNormalsBuffer();
}

void Terrain::onHeightScaleChanged(float newScale)
{
    newScale = std::max(newScale, 0.01f);
    const float invHeightScale = 1 / m_heightScale;
    for (glm::vec3& v : m_vertices) 
            v.y *= invHeightScale * newScale;
    
    m_heightScale = newScale;

    // Update buffers
    generateNormals();

    updateVerticesBuffer();
    updateNormalsBuffer();
}

void Terrain::onNoiseChanged()
{
    const uint32_t width = m_size.x;
    const uint32_t height = m_size.y;
    
    for (uint32_t j = 0; j < height; ++j)
        for (uint32_t i = 0; i < width; ++i)
        {
            uint32_t index = (j * width) + i;
            m_vertices[index].y = m_heightMap[index] * m_heightScale;
        }

    // Update buffers
    generateNormals();

    updateVerticesBuffer();
    updateNormalsBuffer();
}

void Terrain::initRegions()
{
    m_regions = {
        Region("Water Deep",    0.3f,  glm::vec3(0.f, 0.f, 0.8f)),
        Region("Water Shallow", 0.4f,  glm::vec3(54 / 255.f, 103 / 255.f, 199 / 255.f)),
        Region("Sand",          0.45f, glm::vec3(210 / 255.f, 208 / 255.f, 125 / 255.f)),
        Region("Grass",         0.55f, glm::vec3(86 / 255.f, 152 / 255.f, 23 / 255.f)),
        Region("Trees",         0.6f,  glm::vec3(62 / 255.f, 107 / 255.f, 18 / 255.f)),
        Region("Rock",          0.7f,  glm::vec3(90 / 255.f, 69 / 255.f, 60 / 255.f)),
        Region("Higher Rock",   0.9f,  glm::vec3(75 / 255.f, 60 / 255.f, 53 / 255.f)),
        Region("Snow",          1.0f,  glm::vec3(1.f, 1.f, 1.0f)),
    };
}

glm::vec3 Terrain::regionColorIn(float heightValue) const
{
    for (const Region& r : m_regions)
    {
        if (r.toHeight >= heightValue)
            return r.color;
    }
    if (m_regions.size())
        return m_regions.back().color;
    else
        return glm::vec3(1.f, 1.f, 1.f);
}

