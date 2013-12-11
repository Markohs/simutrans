#ifndef TEXTURE_ATLAS_H
#define TEXTURE_ATLAS_H

#include <GL/glew.h>

#include "../display/simimg.h"
#include "../tpl/inthashtable_tpl.h"

#define PALETTE_WIDTH 32
// This is the number of bits to shift PALETTE_WIDTH with in order to get 256
#define PALETTE_SHIFT 3

struct bild_t;

struct texture_info_t {
	GLuint texture, special;
	sint16 x, y;
	sint16 w, h;
	float u1, u2;
	float v1, v2;
	float v_per_pixel;
};

class texture_atlas_t {
public:
	texture_atlas_t();
	~texture_atlas_t();

	void init();

	const texture_info_t *get_texture_info(image_id image) const {
		return &atlas.get(image);
	}

	GLuint get_special_colors() const {
		return special_colors;
	}

	GLubyte get_player_color_offset1(uint8 player) const;
	GLubyte get_player_color_offset2(uint8 player) const;

	// Until geometry classes handle multiple atlases
	const GLuint *get_textures() const {
		return textures;
	}

private:
	void create_special_colors_texture();

	GLsizei num_atlases;
	GLuint *textures;
	GLuint special_colors;
	inthashtable_tpl<image_id, texture_info_t> atlas;
};

extern void register_texture(const bild_t *image);

#endif // TEXTURE_ATLAS_H
