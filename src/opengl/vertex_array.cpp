/**********************************************************
 * < Procedural Terrain Generator >
 * @author Martin Smutny, kentril.despair@gmail.com
 * @date 20.12.2020
 * @file vertex_array.cpp
 * @brief OpenGL Vertex Array Object abstraction
 *********************************************************/

#include "core/pch.hpp"
#include "vertex_array.hpp"


static GLenum element_to_shader_type(ElementType type)
{
    switch(type)
    {
        case ElementType::Float:     return GL_FLOAT;
        case ElementType::Float2:    return GL_FLOAT;
        case ElementType::Float3:    return GL_FLOAT;
        case ElementType::Float4:    return GL_FLOAT;
        case ElementType::Mat3:      return GL_FLOAT;
        case ElementType::Mat4:      return GL_FLOAT;
        case ElementType::Int:       return GL_INT;
        case ElementType::Int2:      return GL_INT;
        case ElementType::Int3:      return GL_INT;
        case ElementType::Int4:      return GL_INT;
        case ElementType::UInt8:     return GL_UNSIGNED_BYTE;
        case ElementType::UInt8_2:   return GL_UNSIGNED_BYTE;
        case ElementType::UInt8_3:   return GL_UNSIGNED_BYTE;
        case ElementType::UInt:      return GL_UNSIGNED_INT;
        case ElementType::UInt2:     return GL_UNSIGNED_INT;
        case ElementType::UInt3:     return GL_UNSIGNED_INT;
        case ElementType::Bool:      return GL_BYTE;
    }
    massert(false, "Unknown buffer element data type!");
    return 0;
}

VertexArray::VertexArray()
{
    glCreateVertexArrays(1, &m_id);
    DERR("VAO default CONSTR: " << m_id);
}

VertexArray::~VertexArray()
{
    DERR("VAO default DESR");
    glDeleteVertexArrays(1, &m_id);

    // TODO Dangerous??
    clear_buffers();
    clear_index();
}

void VertexArray::bind() const
{
    glBindVertexArray(m_id);
}

void VertexArray::unbind() const
{
    glBindVertexArray(0);
}

void VertexArray::add_vertex_buffer(const std::shared_ptr<VertexBuffer>& vbo, 
                                    bool instanced)
{
    const auto& layout = vbo->layout();
    massert(layout.get_elements().size(), "Vertex buffer has no buffer elements!");

    for (const auto& e : layout)
    {
        switch(e.type)
        {
            case ElementType::Float: 
            case ElementType::Float2:
            case ElementType::Float3:
            case ElementType::Float4:
            case ElementType::Int:   
            case ElementType::Int2:  
            case ElementType::Int3:  
            case ElementType::Int4:  
            case ElementType::UInt8:
            case ElementType::UInt8_2:
            case ElementType::UInt8_3:
            case ElementType::UInt: 
            case ElementType::UInt2: 
            case ElementType::UInt3: 
            case ElementType::Bool:  
            {
                // TODO offset x relativeoffset
                glVertexArrayVertexBuffer(m_id, binding_index, vbo->ID(), 
                                          0, layout.get_stride());
                glEnableVertexArrayAttrib(m_id, binding_index);

                glVertexArrayAttribFormat(m_id, binding_index, 
                                          e.components_count(), 
                                          element_to_shader_type(e.type),
                                          e.normalized ? GL_TRUE : GL_FALSE, 
                                          e.offset);
                glVertexArrayAttribBinding(m_id, binding_index, binding_index);

                if (instanced)
                {
                    glBindVertexArray(m_id);
                    glVertexAttribDivisor(binding_index, 1);    // TODO ver 4.5??
                    glBindVertexArray(0);
                }

                binding_index++;
                break;
            }
            case ElementType::Mat3:  
            case ElementType::Mat4:  
            {
                glBindVertexArray(m_id);
                uint8_t count = e.components_count();
                for (uint8_t i = 0; i < count; ++i)
                {
                    glVertexArrayVertexBuffer(m_id, binding_index, vbo->ID(), 
                                              0, layout.get_stride());
                    glEnableVertexArrayAttrib(m_id, binding_index);

                    glVertexArrayAttribFormat(m_id, binding_index, 
                                              count, 
                                              element_to_shader_type(e.type),
                                              e.normalized ? GL_TRUE : GL_FALSE, 
                                              e.offset + i * count * sizeof(float));
                    glVertexArrayAttribBinding(m_id, binding_index, binding_index);
                    // Per instance (GLSL vec4 limitation)
                    glVertexAttribDivisor(binding_index, 1);

                    binding_index++;
                }
                glBindVertexArray(0);
                break;
            }
            default:
                massert(false, "Unknown buffer element data type!");
        }
    }

    vertex_buffers.push_back(vbo);
}

void VertexArray::set_index_buffer(const std::shared_ptr<IndexBuffer>& ibo)
{
    glVertexArrayElementBuffer(m_id, ibo->ID());
    index_buffer = ibo;
}

