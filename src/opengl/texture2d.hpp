#pragma once

#include <glm/glm.hpp>
#include <string>


class Texture2D
{
public:

	/**
	 * @brief Creates an empty 2D texture object.
	 *     	  Defaults: Image format: RGBA, internal format: RGBA8
     *     	  Expects image to be loaded later.
	 */
	Texture2D(bool mipmaps = true);

	Texture2D(const std::string& filename, 
			  bool alpha = true, 
			  bool mipmaps = true); 

	~Texture2D();

	/**
	 * @brief Loads a 2D image as a texture, 
	 * @param filename Name of the texture image
 	 * @param alpha Whether the image has an alpha channel
 	 */
	void load(const std::string& filename, bool alpha = true);

	/**
 	 * @brief Bind the texture object
	 */
	void bind() const;
	
	/**
 	 * @brief Bind the texture to the texture unit
	 * @param unit Number of the texture unit
	 */
	void bind_unit(uint32_t unit) const;

    /**
     * @brief Activates texture unit 'unit' globally.
     */
    void activate(uint32_t unit) const;

    // Default
    void set_repeat();

    void set_mirrored_repeat();

    void set_clamp_to_edge();

    void set_clamp_to_border(const glm::vec4& border_color);

    void set_custom_wrap(uint32_t wrap);

    void set_custom_wrap(uint32_t wrap_s, uint32_t wrap_t);

    void gen_mipmap();

	// Setters
	void set_internal_format(uint32_t f) { internal_format = f; } 

	void set_image_format(uint32_t f) { image_format = f; }

	// Getters
	// TODO each file same
	uint32_t ID() const { return id; }

	glm::uvec2 size() const { return glm::uvec2(width, height); }

private:
		// TODO each file same
	uint32_t id;

	uint32_t width, height;
	uint32_t internal_format, image_format;
	bool mipmaps;
};

