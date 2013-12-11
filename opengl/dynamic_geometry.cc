#include <GL/glew.h>

#include "../simdebug.h"
#include "../simworld.h"

#include "dynamic_geometry.h"

static const size_t MAX_VERTICIES = 4096;
static const size_t MAX_PRIMITIVES = MAX_VERTICIES / 3 - 10;

dynamic_geometry_t::dynamic_geometry_t(karte_t *world, const texture_atlas_t *texture_atlas) : geometry_manager_t(world, texture_atlas), vbo_index(0) {
	work.reserve(MAX_VERTICIES);

	glGenBuffers(2, vbos);
	glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
	glBufferData(GL_ARRAY_BUFFER, MAX_VERTICIES * sizeof(vertex_t), NULL, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbos[1]);
	glBufferData(GL_ARRAY_BUFFER, MAX_VERTICIES * sizeof(vertex_t), NULL, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

dynamic_geometry_t::~dynamic_geometry_t() {
	glDeleteBuffers(2, vbos);
}

void dynamic_geometry_t::display(const matrix_t &view_transform, int vw, int vh, const std::vector<bool> &visibility) {
	const int w = world->get_size().x;
	const int h = world->get_size().y;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_atlas->get_textures()[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture_atlas->get_textures()[1]);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, texture_atlas->get_special_colors());
	enable_array_pointers();

//	glUseProgram(0);
//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//	glColor3f(1.0f, 0.0f, 0.0f);

	size_t primitives = 0;
#ifdef DEBUG
	size_t total_primitives = 0;
#endif

	const int grid_w = (w + BATCH_WH - 1) / BATCH_WH;

	for (size_t b = 0; b < visibility.size(); ++b) {
		if (visibility[b]) {
			int gx = b % grid_w;
			int gy = b / grid_w;
			int sx = gx * BATCH_WH;
			int sy = gy * BATCH_WH;
			int ex = (gx+1) * BATCH_WH;
			int ey = (gy+1) * BATCH_WH;

			for (int y = sy; y < ey && y < h; ++y) {
				for (int x = sx; x < ex && x < w; ++x) {
					const planquadrat_t *tile = world->access(koord(x, y));

					for (unsigned int i = 0; i < tile->get_boden_count(); ++i) {
						const grund_t *base = tile->get_boden_bei(i);

						for (uint8 j = 0; j < base->get_top(); ++j) {
							const obj_t *thing = base->obj_bei(j);
							if (thing) {
								if (thing->is_moving() || thing->is_non_static()) {
									int p = add_object(&work, NULL, base, thing);
									primitives += p;
#ifdef DEBUG
									total_primitives += p;
#endif
								}

								if (primitives >= MAX_PRIMITIVES) {
									primitives = flush(primitives);
								}
							}
						}
					}
				}
			}
		}
	}

	if (primitives > 0) {
		flush(primitives);
	}

//	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	disable_array_pointers();
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	DBG_MESSAGE("dynamic_geometry_t::display", "total_primitives: %d", total_primitives);
}

int dynamic_geometry_t::flush(int num_primitives) {
	int draw_primitives = min(MAX_PRIMITIVES, num_primitives);

	vertex_buffer::iterator first = work.begin();
	vertex_buffer::iterator last = work.begin() + draw_primitives * 3;

	glBindBuffer(GL_ARRAY_BUFFER, vbos[vbo_index]);

	void *ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	std::copy(first, last, static_cast<vertex_t *>(ptr));
	glUnmapBuffer(GL_ARRAY_BUFFER);

	setup_array_pointers();

	glDrawArrays(GL_TRIANGLES, 0, draw_primitives * 3);

	work.erase(first, last);

	vbo_index ^= 1;

	return num_primitives - draw_primitives;
}
