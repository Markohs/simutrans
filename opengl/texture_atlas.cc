#include <cassert>
#include <cstring>
#include <vector>
#include <algorithm>

#include <GL/glew.h>

//#define DUMP_ATLAS
#ifdef DUMP_ATLAS
#include <SDL.h>
#endif

#include "../simdebug.h"
#include "../display/simgraph.h"
#include "../simmem.h"
#include "../simtools.h"
#include "../besch/bild_besch.h"

#include "texture_atlas.h"

#if PALETTE_WIDTH < (16 + LIGHT_COUNT)
#error Palette not wide enough. PALETTE_WIDTH must be increased.
#endif

static std::vector<const bild_t *> images;

extern int closest_pot(const int number);
extern int get_tex_max_size();

static bool compare_image_height(const bild_t *a, const bild_t *b) {
	return max(a->y + a->h, get_base_tile_raster_width()) > max(b->y + b->h, get_base_tile_raster_width());
}

// Copied, trimmed and adapted from simgraph16.cc
typedef uint16 pixel_t;
static void draw_image(const bild_t *image, pixel_t *texture, uint8 *special, uint32 xp, uint32 yp, uint32 disp_width) {
	const uint16 *sp = image->data;
	sint16 h = image->h;

	if (h > 0) {
		pixel_t *tp = texture + xp + image->x + (yp + image->y) * disp_width;
		uint8 *up = special + xp + image->x + (yp + image->y) * disp_width;

		do { // zeilen dekodieren
			uint16 runlen = *sp++;
			pixel_t *p = tp;
			uint8* u = up;

			// eine Zeile dekodieren
			do {
				// Start with clear run
				p += runlen;
				u += runlen;

				// jetzt kommen farbige pixel
				runlen = *sp++;

				while (runlen--) {
					uint16 value = *sp++;
					uint8 special;
					if (value & 0x8000) {
						special = (value & 0xFF) << PALETTE_SHIFT;
						value = 0x8000;
					} else {
						value |= 0x8000;
						special = 0xFF;
					}
					*p++ = value;
					*u++ = special;
				}

				runlen = *sp++;
			} while (runlen != 0);

			tp += disp_width;
			up += disp_width;
		} while (--h > 0);
	}
}

#ifdef DUMP_ATLAS
static void dump_atlas(unsigned int t, pixel_t *pixels, uint32 w, uint32 h) {
	char buf[64];
	SDL_Surface *s = SDL_CreateRGBSurfaceFrom(pixels, w, h, 16, w * sizeof(pixel_t), 0x00007C00, 0x000003E0, 0x0000001F, 0x00008000);
	sprintf(buf, "atlas%u.bmp", t);
	SDL_SaveBMP(s, buf);
	SDL_FreeSurface(s);
}
#endif // DUMP_ATLAS

void register_texture(const bild_t *image) {
	images.push_back(image);
}

texture_atlas_t::texture_atlas_t() : textures(NULL), special_colors(0) {
}

void texture_atlas_t::init() {
	if (textures) {
		glDeleteTextures(num_atlases * 2, textures);
		delete textures;
		textures = NULL;
	}
	create_special_colors_texture();

	std::vector<const bild_t *> sorted_images(images.begin(), images.end());
	std::sort(sorted_images.begin(), sorted_images.end(), compare_image_height);

	const sint32 max_size = get_tex_max_size();

	// Figure out how many textures are needed
	sint32 x = 0;
	sint32 y = 0;
	sint32 line_height = max(sorted_images[0]->y + sorted_images[0]->h, get_base_tile_raster_width());
	num_atlases = 1;
	if (line_height > max_size) {
		dbg->fatal("texture_atlas::texture_atlas()", "Largest image is higher (%u) than texture size (%u)", line_height, max_size);
	}
	for (std::vector<const bild_t *>::iterator iter = sorted_images.begin(); iter != sorted_images.end(); ++iter) {
		const bild_t *image = *iter;
		const sint32 img_w = max(image->x + image->w, get_base_tile_raster_width());
		if (img_w > max_size) {
			dbg->fatal("texture_atlas::texture_atlas()", "Image is wider (%u) than texture size (%u)", image->w, max_size);
		}
		x += img_w;
		if (x > max_size) {
			x = img_w;
			y += line_height;
			line_height = max(image->y + image->h, get_base_tile_raster_width());
			if ((y + line_height) > max_size) {
				dbg->fatal("texture_atlas_t::init()", "No support for multiple atlases (yet), and atlas of %dx%d is full", max_size, max_size);
				++num_atlases;
				y = 0;
			}
		}
	}

	const uint32 last_atlas_height = closest_pot(y + line_height);

	DBG_DEBUG("texture_atlas::texture_atlas()", "allocating %u atlases of size %ux%u/%u", num_atlases, max_size, max_size, last_atlas_height);

	textures = new GLuint[num_atlases * 2];
	glGenTextures(num_atlases * 2, textures);

	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

	const sint32 texture_width = max_size;
	const sint32 texture_height = (num_atlases == 1 ? last_atlas_height : max_size);

	pixel_t *data = MALLOCN(pixel_t, texture_width * texture_height);
	uint8 *special = MALLOCN(uint8, texture_width * texture_height);
	memset(data, 0, sizeof(pixel_t) * texture_width * texture_height);
	memset(special, 0xFF, sizeof(uint8) * texture_width * texture_height);

	int t = 0;
	x = 0;
	y = 0;
	line_height = max(sorted_images[0]->y + sorted_images[0]->h, get_base_tile_raster_width());
	for (std::vector<const bild_t *>::iterator iter = sorted_images.begin(); iter != sorted_images.end(); ++iter) {
		const bild_t *image = *iter;
		const sint32 img_w = max(image->x + image->w, get_base_tile_raster_width());
		if (x + img_w > texture_width) {
			x = 0;
			y += line_height;
			line_height = max(image->y + image->h, get_base_tile_raster_width());

			if (y + line_height > texture_height) {
				glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
				glBindTexture(GL_TEXTURE_2D, textures[t * 2]);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5_A1, texture_width, texture_height, 0, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, data);
#ifdef DUMP_ATLAS
				dump_atlas(t, data, texture_width, texture_height);
#endif

				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
				glBindTexture(GL_TEXTURE_2D, textures[t * 2 + 1]);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, texture_width, texture_height, 0, GL_RED, GL_UNSIGNED_BYTE, special);

				++t;
				memset(data, 0, sizeof(pixel_t) * texture_width * texture_height);
				memset(special, 0xFF, sizeof(uint8) * texture_width * texture_height);
				y = 0;
			}
		}

		draw_image(image, data, special, x, y, texture_width);

		const uint32 actual_texture_height = t < (num_atlases - 1) ? texture_height : last_atlas_height;

		texture_info_t info;
		info.texture = textures[t * 2];
		info.special = textures[t * 2 + 1];
		info.x = image->x;
		info.y = image->y;
		info.w = image->w;
		info.h = image->h;
		info.u1 = float(x) / float(texture_width);
		info.v1 = float(y) / float(actual_texture_height);
		info.u2 = float(x + img_w) / float(texture_width);
		info.v2 = float(y + line_height) / float(actual_texture_height);
		info.v_per_pixel = 1.0 / float(actual_texture_height);
		atlas.put(image->bild_nr, info);

		x += img_w;
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
	glBindTexture(GL_TEXTURE_2D, textures[t * 2]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5_A1, texture_width, last_atlas_height, 0, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, data);
#ifdef DUMP_ATLAS
	dump_atlas(t, data, texture_width, last_atlas_height);
#endif

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glBindTexture(GL_TEXTURE_2D, textures[t * 2 + 1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, texture_width, last_atlas_height, 0, GL_RED, GL_UNSIGNED_BYTE, special);

	glBindTexture(GL_TEXTURE_2D, 0);

	free(data);
	free(special);
}

texture_atlas_t::~texture_atlas_t() {
	if (textures) {
		glDeleteTextures(num_atlases * 2, textures);
		delete textures;
	}
	if (special_colors) {
		glDeleteTextures(1, &special_colors);
		special_colors = 0;
	}
}

static inline pixel_t pack_rgb(COLOR_VAL r, COLOR_VAL g, COLOR_VAL b) {
	return (pixel_t(r & 0xF8) << 7) | (pixel_t(g & 0xF8) << 2) | (pixel_t(b & 0xF8) >> 3) | 0x8000;
}

void texture_atlas_t::create_special_colors_texture() {
	if (special_colors) {
		glDeleteTextures(1, &special_colors);
		special_colors = 0;
	}

	pixel_t *palette = MALLOCN(pixel_t, PALETTE_WIDTH * MAX_PLAYER_COUNT);

	for (size_t i = 0; i < MAX_PLAYER_COUNT; ++i) {
		pixel_t *p = palette + i * PALETTE_WIDTH;

		for (size_t j = 0; j < 8; ++j) {
			size_t offset = player_offsets[i][0] + j;
			p[j] = pack_rgb(special_pal[offset * 3], special_pal[offset * 3 + 1], special_pal[offset * 3 + 2]);
		}

		for (size_t j = 0; j < 8; ++j) {
			size_t offset = player_offsets[i][1] + j;
			p[j + 8] = pack_rgb(special_pal[offset * 3], special_pal[offset * 3 + 1], special_pal[offset * 3 + 2]);
		}

		for (size_t j = 0; j < LIGHT_COUNT; ++j) {
			p[j + 16] = pack_rgb(display_day_lights[j * 3], display_day_lights[j * 3 + 1], display_day_lights[j * 3 + 2]);
		}

		for (size_t j = 16 + LIGHT_COUNT; j < PALETTE_WIDTH; ++j) {
			p[j] = 0;
		}
	}

	glGenTextures(1, &special_colors);
	glBindTexture(GL_TEXTURE_2D, special_colors);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5_A1, PALETTE_WIDTH, MAX_PLAYER_COUNT, 0, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, palette);
	glBindTexture(GL_TEXTURE_2D, 0);

	free(palette);
}
