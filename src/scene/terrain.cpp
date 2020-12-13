#include "core/pch.hpp"
#include "terrain.hpp"

#include <cstring>

#define TRIANGLES_PER_QUAD 2
#define INDICES_PER_TRIANGLE 3

//#define DEBUG_STRIP

Terrain::Terrain(uint32_t x, uint32_t y,
                 float tileScale, float heightScale,
                 const glm::mat4& model)
: m_size(x, y),
  m_tileScale(tileScale),
  m_heightScale(heightScale),
  m_model(model),
  m_invModel(glm::inverse(model)),
  m_heightMap(x),
  m_scaleFactor(1.0f),
  m_surface(true),
  m_useFalloff(true)
{
    init();
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
  m_scaleFactor(1.0f),
  m_surface(true),
  m_useFalloff(true)
{
    init();
}

void Terrain::init()
{
    m_surface.set_internal_format(GL_RGB8);
    m_surface.set_image_format(GL_RGB);
    m_surface.set_clamp_to_edge();
    m_surface.set_filtering(GL_NEAREST, GL_NEAREST);

    glGenTextures(1, &m_colorTextures);
    glGenTextures(1, &m_textureArray);
    glGenTextures(1, &m_opacityMap);

    initRegions();
    initColorTextureArray();
    generateFalloffMap();
    generate();
}

void Terrain::addShader(const std::shared_ptr<Shader>& sh)
{
    if (shSingle == nullptr)
        shSingle = sh;
    else if (shMulti == nullptr)
    {
        shMulti = sh;
        shMulti->use();
        shMulti->set_int("multiTex", 0);
        shMulti->set_int("opacityMap", 1);
        glUseProgram(0);
    }
    // TODO others
}

// TODO break into functions
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
    m_opacities.resize(m_regions.size());

    for (uint32_t i = 0; i < m_regions.size(); ++i)
        m_opacities[i].resize(totalVertices);

    // Calculate size of the terrain in world units
    // The dimensions in world units are [0, ..., size-1] 
    m_worldSize = glm::vec2((width  -1) * m_tileScale, 
                            (height -1) * m_tileScale);

    m_worldHalfSize = glm::vec2(m_worldSize.x * 0.5f, m_worldSize.y * 0.5f);
    LOG_INFO("Terrain world dimensions: " << m_worldSize.x << " x " << m_worldSize.y);

    float S, T, X, Y, Z;
    for (uint32_t y = 0; y < height; ++y)
        for (uint32_t x = 0; x < width; ++x)
        {
            uint32_t index = (y * width) + x;

            // Sample the height - value between <0; 1>
            float heightValue = m_heightMap[index];
            if (m_useFalloff)
                heightValue = glm::clamp(heightValue - m_falloffMap[index], 0.f, 1.f);

            // Texture coords of the terrain from top-left (0,0) to bottom right (1,1)
            S = x / (float)(width -1);
            T = y / (float)(height-1);

            // Compute vertex positions in world units
            // Center the terrain around the local origin to the terrain dimensions:
            //  X: [-m_worldHalfSize.x, .., m_worldHalfSize.x]
            //  Z: [-m_worldHalfSize.y, ..., m_worldHalfSize.y]
            
            X = S * m_worldSize.x - m_worldHalfSize.x;
            Y = heightValue * m_heightScale;
            Z = T * m_worldSize.y - m_worldHalfSize.y;

            m_texCoords[index] = glm::vec2(S, T);
            m_vertices[index] = glm::vec3(X, Y, Z);

            // TODO height-based texturing ADD BOOL or ANOTHER FUNCTION
            regionColorIn(heightValue, index);
        }

    // ---------------------------------------------------------------- 
    // Generate buffers
    generateIndices();
    generateNormals();

    generateBuffers();
    updateTextures();

    LOG_OK("Terrain has been loaded!");
}

void Terrain::updateTextures()
{
    if (m_blending)
    {
        initOpacityMap();
        fillOpacityMap();

        if (m_usingColors)
        {
            //initColorTextureArray();
            fillColorTextureArray();
        }
        else    // Textures
        {
                // Not changing
        }
    }
    else
    {
        m_surface.upload((float*)&m_colors[0], m_size.x, m_size.y);
    }
}

void Terrain::generateIndices()
{
    const uint32_t width = m_size.x;
    const uint32_t height = m_size.y;
    const uint32_t realW = width  -1;
    const uint32_t realH = height -1;
#ifndef TRIANGLE_STRIP
    // 2 triangles for every quad of the terrain mesh
    //  3 indices for each triangle in the terrain mesh
    m_indices.resize(realW * realH * TRIANGLES_PER_QUAD * INDICES_PER_TRIANGLE);

    uint32_t index = 0;
    for (uint32_t j = 0; j < realH; ++j)
    {
        for (uint32_t i = 0; i < realW; ++i)
        {
            uint32_t vertex_index = j * width + i;
            // Top triangle 
            m_indices[index++] = vertex_index;
            m_indices[index++] = vertex_index + width + 1;
            m_indices[index++] = vertex_index + 1;

            // Bottom triangle
            m_indices[index++] = vertex_index;
            m_indices[index++] = vertex_index + width;
            m_indices[index++] = vertex_index + width+ 1;
        }
    } 
#else
    // 2 triangles for every quad of the terrain mesh
    //  with 3 indices for each triangle in the terrain mesh
    //const uint32_t indices = realW * realH * TRIANGLES_PER_QUAD * INDICES_PER_TRIANGLE;
    const uint32_t indices = m_vertices.size() * 2 - 2;

    m_indices.resize(indices);
#ifdef DEBUG_STRIP
    std::cout << "Indices: " << m_indices.size() << std::endl;
#endif

    // TODO better
    uint32_t index = 0;
    for (uint32_t y = 0; y < realH; ++y)
    {
        uint32_t vertexIndex = y * width;
        m_indices[index++] = vertexIndex;
    #ifdef DEBUG_STRIP
        std::cout << vertexIndex;
    #endif
        for (uint32_t x = 0; x < width; ++x)
        {
            m_indices[index++] = vertexIndex + width;
        #ifdef DEBUG_STRIP
            std::cout << ' ' << vertexIndex + width;
        #endif
            if (x < realW)
            {
                m_indices[index++] = vertexIndex + 1;
            #ifdef DEBUG_STRIP
                std::cout << ' ' << vertexIndex + 1;
            #endif
            }

            ++vertexIndex;
        }
        m_indices[index++] = vertexIndex-1 + width;
        m_indices[index++] = (y+1) * width;
    #ifdef DEBUG_STRIP
        std::cout << ' ' << vertexIndex-1 + width << ' ' << (y+1) * width << std::endl;
    #endif
    }
#endif
}

void Terrain::generateNormals()
{
    // Stored continuously
    std::fill(m_normals.begin(), m_normals.end(), glm::vec3(0));

    // Compute normal for each triangle as a cross product of its edges
    glm::vec3 normal;

#ifndef TRIANGLE_STRIP
    for (uint32_t i = 0; i < m_indices.size(); i += 3)
    {
        uint32_t id1 = m_indices[i + 0];
        uint32_t id2 = m_indices[i + 1];
        uint32_t id3 = m_indices[i + 2];
        const glm::vec3& v0 = m_vertices[id1];
        const glm::vec3& v1 = m_vertices[id2];
        const glm::vec3& v2 = m_vertices[id3];

        normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));

        // Save for each vertex
        m_normals[id1] += normal;
        m_normals[id2] += normal;
        m_normals[id3] += normal;
    }
#else
    const uint32_t width = m_size.x;
    const uint32_t height = m_size.y;
    const uint32_t realH = height -1;

    uint32_t i = 0;
    for (uint32_t y = 0; y < realH; ++y)
    {
        for (uint32_t x = 0; x < width+1; ++x)
        {
            uint32_t id1 = m_indices[i + 0];
            uint32_t id2 = m_indices[i + 1];
            uint32_t id3 = m_indices[i + 2];
    #ifdef DEBUG_STRIP
            std::cout << id1 << ' ' << id2 << ' ' << id3 << ' ';
    #endif
            const glm::vec3& v0 = m_vertices[id1];
            const glm::vec3& v1 = m_vertices[id2];
            const glm::vec3& v2 = m_vertices[id3];

            normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));

            m_normals[id1] += normal;
            m_normals[id2] += normal;
            m_normals[id3] += normal;

            ++i;
        }
    #ifdef DEBUG_STRIP
        std::cout << std::endl;
    #endif
        i += 4;
    }
#endif
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

    std::shared_ptr ibo = std::make_shared<IndexBuffer>(m_indices.size(), m_indices.data());

    m_vboVertices->set_layout(BufferLayout({{ElementType::Float3, "Position"}}));
    m_vboNormals->set_layout(BufferLayout({{ElementType::Float3, "Normal"}}));
    m_vboTexels->set_layout(BufferLayout({{ElementType::Float2, "TexCoords"}}));

    m_vao.add_vertex_buffer(m_vboVertices);
    m_vao.add_vertex_buffer(m_vboNormals);
    m_vao.add_vertex_buffer(m_vboTexels);
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

void Terrain::render(const glm::mat4& projView) const
{
    // Disable lighting because it changes the primary color of the vertices that are
    // used for the multitexture blending.

    // TODO better
    if (m_blending)
    {
        //glEnable(GL_BLEND);
        shMulti->use();
        shMulti->set_mat4("MVP", projView * m_model);
        shMulti->set_int("layers", m_regions.size());
        shMulti->set_vec2("scale", m_scaleFactor, m_scaleFactor);


        glActiveTexture(GL_TEXTURE0);
        if (m_usingColors)
            glBindTexture(GL_TEXTURE_2D_ARRAY, m_colorTextures);
        else
        {
            glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureArray);
        }

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D_ARRAY, m_opacityMap);
        //glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); 
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else
    {
        shSingle->use();
        shSingle->set_mat4("MVP", projView * m_model);
        shSingle->set_vec2("scale", m_scaleFactor, m_scaleFactor);
        m_surface.bind();
    }

    m_vao.bind();

    // Rendering using indexed element arrays
#ifndef TRIANGLE_STRIP
    glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, nullptr);
    //glDisable(GL_BLEND);
#else
    glDrawElements(GL_TRIANGLE_STRIP, m_indices.size(), GL_UNSIGNED_INT, nullptr);
#endif
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
    if (newSize == m_size && !m_falloffChanged)
        return;

    m_size = newSize;

    m_vao.unbind();
    m_vao.clear();

    m_heightMap.setSize(m_size.x);
    generateFalloffMap();
    generate();

    m_falloffChanged = false;
}

void Terrain::onTileScaleChanged(float newScale)
{
    if (newScale == m_tileScale)
        return;

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
    if (newScale == m_heightScale)
        return;

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
    if (!m_heightMap.changed())  
        return;

    DERR("Update height map");

    const uint32_t width = m_size.x;
    const uint32_t height = m_size.y;
    
    for (uint32_t y = 0; y < height; ++y)
        for (uint32_t x = 0; x < width; ++x)
        {
            uint32_t index = y * width + x;
            float heightValue = m_heightMap[index]; 
            if (m_useFalloff)
                heightValue = glm::clamp(heightValue - m_falloffMap[index], 0.f, 1.f);

            m_vertices[index].y = heightValue * m_heightScale;
            regionColorIn(heightValue, index);
        }

    // Update buffers
    generateNormals();

    updateVerticesBuffer();
    updateNormalsBuffer();
    updateTextures();
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

float getPercent(const float min, const float max, const float v) noexcept
{
    return glm::clamp((v - min) / (max - min), 0.0f, 1.0f);
}

void Terrain::regionColorIn(float heightValue, uint32_t index)
{
    if (m_blending)
    {
        //TODO check region size

        const float spacing = 0.5f;
        uint32_t i;
        for ( i = 0 ; i < m_regions.size(); ++i)
        {
            const Region& r = m_regions[i];
            if (heightValue <= r.toHeight)
            {
                if (i == m_regions.size()-1)
                {
                    m_opacities[i][index] = 1.0f;
                    i++;
                    break;
                }

                const float startHeight = i ? m_regions[i-1].toHeight : 0.0f;
                const float spaceLen = spacing * (r.toHeight - startHeight);
                
                // interpolate between
                const float startBlend = r.toHeight - spaceLen;
                const float endBlend = r.toHeight + spaceLen;
                float opacity = 1.0f - getPercent(startBlend, endBlend, heightValue);

                m_opacities[i][index] = opacity;
                m_opacities[++i][index] = 1.0-opacity;
                ++i;
                break;
            }
            else
                m_opacities[i][index] = 0.0f;
        }
        // Assign zero to rest
        for ( ; i < m_regions.size(); ++i)
            m_opacities[i][index] = 0.0f;

#ifdef CHECK_HOLDS
        float total = 0;
        for (uint32_t j = 0; j < m_regions.size(); ++j)
            total += m_opacities[j][index];
        
        assert(total == 1.0f);
#endif
    }
    else
    {
        for (const Region& r : m_regions)
        {
            if (r.toHeight >= heightValue)
            {
                m_colors[index] = r.color;
                return;
            }
        }
        if (m_regions.size())
            m_colors[index] = m_regions.back().color;
        else
            m_colors[index] = glm::vec3(1.f, 1.f, 1.f);
    }
}

void Terrain::onRegionsChanged()
{
    const uint32_t width = m_size.x;
    const uint32_t height = m_size.y;
    
    for (uint32_t y = 0; y < height; ++y)
        for (uint32_t x = 0; x < width; ++x)
        {
            uint32_t index = y * width + x;
            float heightValue = m_heightMap[index];
            if (m_useFalloff)
                heightValue = glm::clamp(heightValue - m_falloffMap[index], 0.f, 1.f);
            regionColorIn(heightValue, index);
        }

    updateTextures();
}

void Terrain::generateFalloffMap()
{
    m_falloffMap.resize(m_size.x * m_size.y);

    const float sizeX = m_size.x;
    const float sizeY = m_size.y;
    for (uint32_t i = 0; i < m_size.y; ++i)
        for (uint32_t j = 0; j < m_size.x; ++j)
        {
            float x = i / sizeY * 2 -1;
            float y = j / sizeX * 2 -1;
            m_falloffMap[i * m_size.x + j] = fade(std::max(fabs(x), fabs(y)));
        }
}

void Terrain::initColorTextureArray()
{
    DERR("Initializing color map");
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_colorTextures);

    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 
                   1,                   // levels
                   GL_RGB8, 
  	               colorTexSize, colorTexSize,
                   m_regions.size());

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    DERR("  Done");
}

void Terrain::fillColorTextureArray()
{
    DERR("Filling color map");
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_colorTextures);

    for (uint32_t i = 0; i < m_regions.size(); ++i)
    {
        subColorAt(i);
    }
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    DERR("  Done");
}

void Terrain::subColorAt(uint32_t i)
{
    std::vector<glm::vec3> rcolors(colorTexSize * colorTexSize, 
                                   glm::vec3(m_regions[i].color));
    //DERR(" Adding color: " << rcolors[i].x << ' ' << rcolors[i].y << ' ' << rcolors[i].z);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
                    0,                          // level
                    0, 0, i,                    // offset
  	                colorTexSize, colorTexSize,
                    1,
                    GL_RGB, 
                    GL_FLOAT, 
                    rcolors.data());
                    //m_colors[i].data());
}

/*
void Terrain::initColorTextureArray()
{
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_colorTextures);

    glTexImage3D(GL_TEXTURE_2D_ARRAY,
  	             0,                         // level,
  	             GL_RGB,                  //internalformat, expects Vec3,
                                            // Last is the opacity map alpha
  	             colorTexSize, colorTexSize,
                 m_regions.size(),          // Number of textures
                 0,                         // border
                 GL_RGB,                    // format
  	             GL_FLOAT,                  // type
  	             NULL);                     // data

    //glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, m_size.x, m_size.y, m_regions.size());

}

void Terrain::fillColorTextureArray()
{
    // BOUND BEFORE!

    for (uint32_t i = 0; i < m_regions.size(); ++i)
    {
        subColorAt(i);
    }
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

*/

void Terrain::initTextureArray()
{
    DERR("Initializing texture map");
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureArray);

    const glm::uvec2 maxSize = maxSizeFromImages();
    DERR("Max texture size: " << maxSize.x << " x " << maxSize.y);

    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 
                   1,           // TODO gen mipmaps mipLevelCount
                   GL_RGB8,  // TODO
                   maxSize.x, maxSize.y,
                   m_images.size());

    glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    // TODO GL_REPEAT
    glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_T,GL_REPEAT);

    /* TODO mipmaps
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, format, width, height, num_layers, ...);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 1, format, width/2, height/2, num_layers, ...);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 2, format, width/4, height/4, num_layers, ...);    
    */
        //glTextureParameteri(ID, GL_TEXTURE_WRAP_S, GL_REPEAT);
        //glTextureParameteri(ID, GL_TEXTURE_WRAP_T, GL_REPEAT);
        //glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    for (uint32_t i = 0; i < m_images.size(); ++i)
    {
        const ImageInfo& imgI = m_images[i];
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
                        0,                          // level
                        0, 0, i,                    // offset
                        imgI.width, imgI.height,
                        1,
                        GL_RGB,                 // TODO format
  	                    GL_UNSIGNED_BYTE,
                        imgI.data);
    }
    DERR("  Done");
}

glm::uvec2 Terrain::maxSizeFromImages()
{
    uint32_t maxWidth = 0, maxHeight = 0;
    for (const ImageInfo& i : m_images)
    {
        if (i.width > maxWidth)
            maxWidth = i.width;
        if (i.height > maxHeight)
            maxHeight = i.height;
    }
    return glm::uvec2(maxWidth, maxHeight);
}


void Terrain::setFilteringPoint()
{
    DERR("Setting point filtering");
    if (m_blending)
    {
        glBindTexture(GL_TEXTURE_2D_ARRAY, m_colorTextures);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER,GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D_ARRAY, m_opacityMap);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER,GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureArray);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER,GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    }
    else
        m_surface.set_filtering(GL_NEAREST, GL_NEAREST);
}

void Terrain::setFilteringLinear()
{
    DERR("Setting linear filtering");
    if (m_blending)
    {
        glBindTexture(GL_TEXTURE_2D_ARRAY, m_colorTextures);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureArray);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D_ARRAY, m_opacityMap);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    }
    else
        m_surface.set_linear_filtering();
}

void Terrain::loadTexture(const char* name)
{
    int w, h, channels;
    // TODO FREE LOADED DATA in DESTR
    uint8_t* data = load_image(name, false, &w, &h, &channels);

    m_images.push_back(ImageInfo(data, w, h));
    guiTextures.reserve(m_regions.size());
    guiTextures.push_back(std::make_unique<Texture2D>(data, w, h, false, true));
}

void Terrain::initOpacityMap()
{
    DERR("Init opacity map");
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_opacityMap);

    glTexImage3D(GL_TEXTURE_2D_ARRAY,
  	             0,                         // level,
  	             GL_R8,                    //internalformat, expects ALPHA ONLY
                                            // Last is the opacity map alpha
                 m_size.x, m_size.y,
                 m_regions.size(),          // Number of textures
                 0,                         // border
                 GL_RED,                    // format
  	             GL_FLOAT,                  // type
  	             NULL);                     // data

    //glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, m_size.x, m_size.y, m_regions.size());
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    DERR("  Done");
}

void Terrain::fillOpacityMap()
{
    DERR("Filling opacity map");
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_opacityMap);

    for (uint32_t i = 0; i < m_regions.size(); ++i)
    {

        glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
                        0,                          // level
                        0, 0, i,                    // offset
                        m_size.x, m_size.y,
                        1,
                        GL_RED, 
                        GL_FLOAT, 
                        m_opacities[i].data());
    }

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    DERR("  Done");
}


/*
void fillTextureArray()
{
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_textures);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 
                   mipLevelCount, 
                   GL_RGBA8, 
                   width, 
                   height, 
                   layerCount);

    glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);

    glTexImage3D(GL_TEXTURE_2D_ARRAY,
  	             GLint level,
  	             GLint internalformat,
  	             GLsizei width,
  	             GLsizei height,
  	             GLsizei depth,
  	             GLint border,
  	             GLenum format,
  	             GLenum type,
  	             const void * data);

    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 
                    0, 
                    0, 
                    0, 
                    i, 
                    image.width, 
                    image.height, 
                    1, 
                    textureFormat, 
                    GL_UNSIGNED_BYTE, 
                    image.pixelData);
}
*/
