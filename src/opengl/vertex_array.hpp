#pragma once

#include "buffer.hpp"
#include <vector>
#include <memory>


/**
 * @brief Vertex Array Object interface, mirrors the principle of VAO
 *  that is, it has only references to assigned buffers.
 *  Uses already defined BufferLayout interface from buffer.hpp. 
 *  For buffer usage examples see buffer.hpp.
 *
 *  Usage example:
 *    1) 2 VBOs for vertices and colors and an indexbuffer: 
 *
 *      // setup VBOs and IBO
 *      shared_ptr<VertexBuffer> vbo_pos = make_shared<VertexBuffer>(sizeof(vertices), vertices);
 *      shared_ptr<VertexBuffer> vbo_color = make_shared<VertexBuffer>(sizeof(colors), colors);
 *      shared_ptr<IndexBuffer> ibo = make_shared<IndexBuffer>(
 *          sizeof(indices) / sizeof(uint32_t), indices 
 *      );
 *
 *      // setup VBOs' layouts
 *      {
 *          BufferLayout layout = {
 *              { ElementType::Float3, "Position"}
 *          };
 *          vbo_pos->set_layout(layout);
 *      }
 *      vbo_color->set_layout(BufferLayout({
 *          { ElementType::Float4, "Color"}})
 *      );
 *
 *      // setup VAO
 *      VertexArray vao;
 *
 *      vao.add_vertex_buffer(vao_pos);
 *      vao.add_vertex_buffer(vao_color);
 *      vao.set_index_buffer(ibo);
 *
 *      vao.bind();
 *      // example draw call
 *      glDrawElements(GL_TRIANGLES, vao.get_index_buffer()->get_count(), 
 *                     GL_UNSIGNED_INT, (void*)0);
 *
 */
class VertexArray
{
public:
    VertexArray();
    ~VertexArray();

    void bind() const;
    void unbind() const;

    void add_vertex_buffer(const std::shared_ptr<VertexBuffer>& vbo, bool instanced = false);
    void set_index_buffer(const std::shared_ptr<IndexBuffer>& ibo);

    const uint32_t ID() { return id; }

    bool has_vertex_buffers() const { return !vertex_buffers.empty(); }

    size_t total_buffers() const { return vertex_buffers.size(); }

    const std::vector<std::shared_ptr<VertexBuffer>>& get_buffers() const 
    { 
        return vertex_buffers; 
    }

    const std::shared_ptr<IndexBuffer>& get_index_buffer() const 
    { 
        return index_buffer;
    }

private:
    uint32_t id;
    uint32_t binding_index = 0;

    std::vector<std::shared_ptr<VertexBuffer>> vertex_buffers;
    std::shared_ptr<IndexBuffer> index_buffer;
};