/**********************************************************
 * < Procedural Terrain Generator >
 * @author Martin Smutny, kentril.despair@gmail.com
 * @date 20.12.2020
 * @file buffer.cpp
 * @brief OpenGL Vertex Buffer Object and Index Buffer
 *        Object abstractions
 *********************************************************/

#include "core/pch.hpp"
#include "buffer.hpp"


VertexBuffer::VertexBuffer()
{
    glCreateBuffers(1, &m_id);
}

VertexBuffer::VertexBuffer(uint32_t size, bool immutable)
{
    glCreateBuffers(1, &m_id);
    if (immutable)
        glNamedBufferStorage(m_id, size, nullptr, GL_DYNAMIC_STORAGE_BIT);
    else
        glNamedBufferData(m_id, size, nullptr, GL_DYNAMIC_DRAW);
}

VertexBuffer::VertexBuffer(uint32_t size, const void* data, bool immutable)
{
    glCreateBuffers(1, &m_id);
    if (immutable)
        glNamedBufferStorage(m_id, size, data, GL_DYNAMIC_STORAGE_BIT);
    else
        glNamedBufferData(m_id, size, data, GL_STATIC_DRAW);
}

VertexBuffer::~VertexBuffer()
{
    glDeleteBuffers(1, &m_id);
}

void VertexBuffer::bind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, m_id);
}

void VertexBuffer::unbind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBuffer::set_data(uint32_t size, const void* data, int32_t offset) const
{
    glNamedBufferSubData(m_id, offset, size, data);
}

void VertexBuffer::reallocate(uint32_t size, const void* data, bool updates) const
{
    if (updates)
        glNamedBufferData(m_id, size, data, GL_DYNAMIC_DRAW);
    else
        glNamedBufferData(m_id, size, data, GL_STATIC_DRAW);
}

// ----------------------------------------------------------------------------
// Index Buffer
// ----------------------------------------------------------------------------

IndexBuffer::IndexBuffer(uint32_t count, const uint32_t* indices)
  : m_count(count)
{
    glCreateBuffers(1, &m_id);

    // GL_ELEMENT_ARRAY_BUFFER is not valid without an actively bound VAO
    glNamedBufferStorage(m_id, count * sizeof(uint32_t), indices, GL_DYNAMIC_STORAGE_BIT);
}

IndexBuffer::~IndexBuffer()
{
    glDeleteBuffers(1, &m_id);
}

void IndexBuffer::bind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
}

void IndexBuffer::unbind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

