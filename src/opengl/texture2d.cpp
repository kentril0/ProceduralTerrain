#include "core/pch.hpp"
#include "texture2d.hpp"


Texture2D::Texture2D(bool mps)
    : m_width(0), 
      m_height(0), 
      m_internal_format(GL_RGBA8),
      m_image_format(GL_RGBA),
      m_mipmaps(mps),
      m_filterMin(GL_LINEAR_MIPMAP_LINEAR),
      m_filterMag(GL_LINEAR)
{
    glCreateTextures(GL_TEXTURE_2D, 1, &id);
}

Texture2D::Texture2D(const std::string& filename, bool alpha, bool mps)
    : m_width(0), 
      m_height(0), 
      m_internal_format(GL_RGBA8),
      m_image_format(alpha ? GL_RGBA : GL_RGB),
      m_mipmaps(mps),
      m_filterMin(GL_LINEAR_MIPMAP_LINEAR),
      m_filterMag(GL_LINEAR)
{
    DERR("Texture def CONSTR");
    glCreateTextures(GL_TEXTURE_2D, 1, &id);

    load(filename, alpha);
}

Texture2D::~Texture2D()
{
    DERR("Texture def DESTR");
    glDeleteTextures(1, &id);
}

void Texture2D::load(const std::string& filename, bool alpha)
{
    int w, h, channels;
    unsigned char *data = load_image(filename.c_str(), alpha, &w, &h, &channels);

    // Save image dimensions
    m_width = w;
    m_height = h;

    // Specify IMMUTABLE storage for all levels of a 2D array texture
    //  Create texture with log2 of width levels
    glTextureStorage2D(id,
                       (m_mipmaps ? std::log2(m_width) : 1),    // number of texture levels
                       m_internal_format,           // sized format of stored data *RGBA8
                       m_width, m_height);          // in texels

    // Specify a 2D texture subimage
    glTextureSubImage2D(id,                 // texture id
                        0,                  // level
                        0, 0,               // xoffset, yoffset in the texture array
                        m_width, m_height,  // width, height of the subimage
                        m_image_format,     // format of the pixel data *RED, RGB, RGBA
                        GL_UNSIGNED_BYTE,   // data type of the pixel data, *BYTE, FLOAT, INT
                        data);              // A pointer to the image data in memory

    free_image_data(data);

    // Generate Mipmaps
    if (m_mipmaps)
    	gen_mipmap();
}

void Texture2D::upload(const uint8_t* data, int w, int h)
{
    // Save the dimensions
    m_width = w;
    m_height = h;

    bind();
    // Specify storage for all levels of a 2D array texture
    glTexImage2D(GL_TEXTURE_2D,         // texture type
  	             0,                     // level
  	             m_internal_format,     // sized internal format
  	             m_width, m_height,     // dimensions
                 0,                     // border
                 m_image_format,        // image format
                 GL_UNSIGNED_BYTE,      // image datatype
                 data);                 // pointer to the image data

    // Generate Mipmaps
    if (m_mipmaps)
    	gen_mipmap();

    unbind();
}

void Texture2D::upload(const float* data, int w, int h)
{
    // Save the dimensions
    m_width = w;
    m_height = h;

    bind();
    // Specify storage for all levels of a 2D array texture
    glTexImage2D(GL_TEXTURE_2D,         // texture type
  	             0,                     // level
  	             m_internal_format,     // sized internal format
  	             m_width, m_height,     // dimensions
                 0,                     // border
                 m_image_format,        // image format
                 GL_FLOAT,              // image datatype
                 data);                 // pointer to the image data

    // Generate Mipmaps
    if (m_mipmaps)
    	gen_mipmap();

    unbind();
}

void Texture2D::bind() const
{
    glBindTexture(GL_TEXTURE_2D, id);
}

void Texture2D::unbind() const
{
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::bind_unit(uint32_t unit) const
{
    glBindTextureUnit(unit, id);
}

void Texture2D::set_repeat()
{
    set_custom_wrap(GL_REPEAT);
}

void Texture2D::set_mirrored_repeat()
{
    set_custom_wrap(GL_MIRRORED_REPEAT);
}

void Texture2D::set_clamp_to_edge()
{
	set_custom_wrap(GL_CLAMP_TO_EDGE);
}

void Texture2D::set_clamp_to_border(const glm::vec4& border_color)
{
	set_custom_wrap(GL_CLAMP_TO_BORDER);
	glTextureParameterfv(id, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(border_color));
}

void Texture2D::set_custom_wrap(uint32_t wrap)
{
    glTextureParameteri(id, GL_TEXTURE_WRAP_S, wrap);
    glTextureParameteri(id, GL_TEXTURE_WRAP_T, wrap);	
}

void Texture2D::set_custom_wrap(uint32_t wrap_s, uint32_t wrap_t)
{
    glTextureParameteri(id, GL_TEXTURE_WRAP_S, wrap_s);
    glTextureParameteri(id, GL_TEXTURE_WRAP_T, wrap_t);	
}

void Texture2D::gen_mipmap()
{
	glGenerateTextureMipmap(id);

    glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, m_filterMin);
    glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, m_filterMag);
}

void Texture2D::set_filtering(uint32_t min_f, uint32_t mag_f)
{
    glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, min_f);
    glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, mag_f);
    m_filterMin = min_f;
    m_filterMag = mag_f;
}

void Texture2D::set_linear_filtering()
{
    // Returns the weighted average of the four texture elements that are closest 
    //  to the specified texture coordinate
    set_filtering(GL_LINEAR, GL_LINEAR);
    m_filterMin = GL_LINEAR;
    m_filterMag = GL_LINEAR;
}

void Texture2D::activate(uint32_t unit) const
{
    if (unit > 80)
        LOG_WARN("Going over of the ActiveTexture maximum units supported");

    glActiveTexture(GL_TEXTURE0 + unit);
}

