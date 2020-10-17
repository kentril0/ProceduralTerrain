#include "core/pch.hpp"
#include "texture2d.hpp"


Texture2D::Texture2D(bool mps)
    : width(0), 
      height(0), 
      internal_format(GL_RGBA8),
      image_format(GL_RGBA),
      mipmaps(mps)
{
    glCreateTextures(GL_TEXTURE_2D, 1, &id);
}

Texture2D::Texture2D(const std::string& filename, bool alpha, bool mps)
    : width(0), 
      height(0), 
      internal_format(GL_RGBA8),
      image_format(alpha ? GL_RGBA : GL_RGB),
      mipmaps(mps)
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
    width = w;
    height = h;

    // Create texture with log2 of width levels
    glTextureStorage2D(id,
                       (mipmaps ? std::log2(width) : 1),
                       internal_format,
                       width, height);

    glTextureSubImage2D(id,
                        0,                         //
                        0, 0,                      //
                        width, height,             //
                        image_format, GL_UNSIGNED_BYTE, //
                        data);

    free_image_data(data);

    // Generate Mipmaps
    if (mipmaps)
    	gen_mipmap();
}

void Texture2D::bind() const
{
    glBindTexture(GL_TEXTURE_2D, id);
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

    glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Texture2D::activate(uint32_t unit) const
{
    if (unit > 80)
        LOG_WARN("Going over of the ActiveTexture maximum units supported");

    glActiveTexture(GL_TEXTURE0 + unit);
}

