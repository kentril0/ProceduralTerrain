#include "core/pch.hpp"
#include "buffer.hpp"


VertexBuffer::VertexBuffer()
{
    glCreateBuffers(1, &ID);
}

VertexBuffer::VertexBuffer(uint32_t size, bool immutable)
{
    glCreateBuffers(1, &ID);
    if (immutable)
        glNamedBufferStorage(ID, size, nullptr, GL_DYNAMIC_STORAGE_BIT);
    else
        glNamedBufferData(ID, size, nullptr, GL_DYNAMIC_DRAW);
}

VertexBuffer::VertexBuffer(uint32_t size, const void* data, bool immutable)
{
    glCreateBuffers(1, &ID);
    if (immutable)
        glNamedBufferStorage(ID, size, data, GL_DYNAMIC_STORAGE_BIT);
    else
        glNamedBufferData(ID, size, data, GL_STATIC_DRAW);
}

VertexBuffer::~VertexBuffer()
{
    glDeleteBuffers(1, &ID);
}

void VertexBuffer::bind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, ID);
}

void VertexBuffer::unbind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBuffer::set_data(uint32_t size, const void* data, int32_t offset) const
{
    glNamedBufferSubData(ID, offset, size, data);
}

void VertexBuffer::reallocate(uint32_t size, const void* data, bool updates) const
{
    if (updates)
        glNamedBufferData(ID, size, data, GL_DYNAMIC_DRAW);
    else
        glNamedBufferData(ID, size, data, GL_STATIC_DRAW);
}

// ----------------------------------------------------------------------------
// Index Buffer
// ----------------------------------------------------------------------------

IndexBuffer::IndexBuffer(uint32_t count, const uint32_t* indices)
  : count(count)
{
    glCreateBuffers(1, &ID);

    // GL_ELEMENT_ARRAY_BUFFER is not valid without an actively bound VAO
    glNamedBufferStorage(ID, count * sizeof(uint32_t), indices, GL_DYNAMIC_STORAGE_BIT);
}

IndexBuffer::~IndexBuffer()
{
    glDeleteBuffers(1, &ID);
}

void IndexBuffer::bind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
}

void IndexBuffer::unbind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

