#include <climits>
#include <stdio.h>
#include <GL/glew.h>

#if _WIN32
#define snprintf sprintf_s
#endif

#include "../simdebug.h"
#include "../simworld.h"
#include "../besch/grund_besch.h"

#include "static_geometry.h"

//#define DUMP_BATCHES

bounding_box_t::bounding_box_t() {
	min_x = min_y = min_z = INT_MAX;
	max_x = max_y = max_z = INT_MIN;
}

void bounding_box_t::include(int x, int y, int z_min, int z_max) {
	min_x = min(x, min_x);
	min_y = min(y, min_y);
	min_z = min(z_min, min_z);
	max_x = max(x+1, max_x);
	max_y = max(y+1, max_y);
	max_z = max(z_max, max_z);
}

static_geometry_t::static_geometry_t(karte_t *world, const texture_atlas_t *texture_atlas) : geometry_manager_t(world, texture_atlas), batches(NULL), batch_infos(NULL) {
	work.reserve(BATCH_WH * BATCH_WH * 6);
}

static_geometry_t::~static_geometry_t() {
	if (batches) {
		glDeleteBuffers(grid_w * grid_h, batches);
		delete[] batches;
		delete[] batch_infos;
	}
}

void static_geometry_t::reset() {
	if (batches) {
		glDeleteBuffers(grid_w * grid_h, batches);
		delete[] batches;
		batches = NULL;
		delete[] batch_infos;
		batch_infos = NULL;
	}
}

void static_geometry_t::display(const matrix_t &view_transform, int vw, int vh) {
	if (!batches) {
		regenerate();
	}

	do_dirty();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_atlas->get_textures()[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture_atlas->get_textures()[1]);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, texture_atlas->get_special_colors());
	enable_array_pointers();

	for (int j = 0; j < grid_h; ++j) {
		for (int i = 0; i < grid_w; ++i) {
			const int index = j * grid_w + i;
			const batch_info_t &bi = batch_infos[index];

			float min_x, min_y, max_x, max_y;

			view_transform.transform(bi.bbox.min_x, bi.bbox.max_y, 0.0f, &min_x, NULL, NULL);
			view_transform.transform(bi.bbox.max_x, bi.bbox.min_y, 0.0f, &max_x, NULL, NULL);
			view_transform.transform(bi.bbox.min_x, bi.bbox.min_y, bi.bbox.max_z, NULL, &min_y, NULL);
			view_transform.transform(bi.bbox.max_x, bi.bbox.max_y, bi.bbox.min_z, NULL, &max_y, NULL);

			// TODO: Viewport truncated for debugging purposes
			const int shrinkage_x = vw / 4;
			const int shrinkage_y = vh / 4;
			if (min_x < (vw - shrinkage_x) && max_x >= shrinkage_x && -min_y < (vh - shrinkage_y) && -max_y >= shrinkage_y) {
				glBindBuffer(GL_ARRAY_BUFFER, batches[index]);

				setup_array_pointers();

				glDrawArrays(GL_TRIANGLES, 0, bi.ground_primitives * 3 /* + bi.underground_primitives * 3 */);

				visible[index] = true;
			}
			else {
				visible[index] = false;
			}
		}
	}

	disable_array_pointers();
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void static_geometry_t::regenerate() {
	if (batches) {
		glDeleteBuffers(grid_w * grid_h, batches);
		delete[] batches;
		batches = NULL;
		delete[] batch_infos;
	}
	dirty.clear();
	visible.clear();

	grid_w = (world->get_size().x + BATCH_WH - 1) / BATCH_WH;
	grid_h = (world->get_size().y + BATCH_WH - 1) / BATCH_WH;

	int num_batches = grid_w * grid_h;
	if (num_batches > 0) {
		batches = new GLuint[num_batches];
		batch_infos = new batch_info_t[num_batches];

		glGenBuffers(num_batches, batches);

		dirty.resize(num_batches, true);
		visible.resize(num_batches, false);
	}

	DBG_DEBUG("batch_grid::regenerate()", "grid_w=%d, grid_h=%d\n", grid_w, grid_h);
}

void static_geometry_t::do_dirty() {
	for (int j = 0; j < grid_h; ++j) {
		for (int i = 0; i < grid_w; ++i) {
			int index = j * grid_w + i;

			if (dirty[index]) {
				recreate_batch(i, j);
				dirty[index] = false;
			}
		}
	}
}

void static_geometry_t::recreate_batch(int i, int j) {
	DBG_DEBUG("batch_grid::recreate_batch()", "i=%d, j=%d\n", i, j);

	work.clear();
	work_underground.clear();

	const int ix = i * BATCH_WH;
	const int iy = j * BATCH_WH;
	const int ex = ix + BATCH_WH;
	const int ey = iy + BATCH_WH;
	const int wx = world->get_size().x;
	const int wy = world->get_size().y;

	const int batch_index = j * grid_w + i;
	batch_info_t * const batch_info = &batch_infos[batch_index];
	batch_info->ground_primitives = 0;
	batch_info->underground_primitives = 0;

	for (int y = iy; y < ey && y < wy; ++y) {
		for (int x = ix; x < ex && x < wx; ++x) {
			const planquadrat_t *pq = world->access(koord(x, y));
			const grund_t *ground = pq->get_kartenboden();
			batch_info->ground_primitives += add_tile(&work, &batch_info->bbox, ground);

			for (unsigned int l = 1; l < pq->get_boden_count(); ++l) {
				const grund_t *base = pq->get_boden_bei(l);
				if (base->ist_tunnel()) {
					batch_info->underground_primitives += add_tile(&work_underground, &batch_info->bbox, base);
				}
				else {
					batch_info->ground_primitives += add_tile(&work, &batch_info->bbox, base);
				}
			}
		}
	}

	work.insert(work.end(), work_underground.begin(), work_underground.end());

	glBindBuffer(GL_ARRAY_BUFFER, batches[batch_index]);

	glBufferData(GL_ARRAY_BUFFER, work.size() * sizeof(vertex_t), &work[0], GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

#ifdef DUMP_BATCHES
	dump(work, *batch_info, i, j);
#endif
}

static float calc_v1(const texture_info_t *ti, int z, int z_se, int tile_height_step) {
	const float v_z = ti->v_per_pixel * tile_height_step;
//	const float v_half = get_base_tile_raster_width() * ti->v_per_pixel / 4.0f;

	if (z_se == z) {
		return ti->v2;
	}
	else if (z_se < z) { // Should not happen
		return ti->v2 + v_z;
	}
	else {
		return ti->v2 - v_z;
	}
}

static float calc_v2(const texture_info_t *ti, int z, int z_nw, int tile_height_step) {
	const float v_z = ti->v_per_pixel * tile_height_step;
	const float v_half = get_base_tile_raster_width() * ti->v_per_pixel / 4.0f;

	if (z_nw == z) {
		return ti->v2 - 2 * v_half;
	}
	else if (z_nw < z) { // Should not happen?
		return ti->v2 - 2 * v_half + v_z;
	}
	else {
		return ti->v2 - 2 * v_half - v_z;
	}
}

static float calc_v3(const texture_info_t *ti, int z, int z2, int tile_height_step) {
	const float v_z = ti->v_per_pixel * tile_height_step;
	const float v_half = get_base_tile_raster_width() * ti->v_per_pixel / 4.0f;

	if (z2 == z) {
		return ti->v2 - v_half;
	}
	else if (z2 < z) { // Should not happen?
		return ti->v2 - v_half + v_z;
	}
	else {
		return ti->v2 - v_half - v_z;
	}
}

size_t static_geometry_t::add_tile(std::vector<vertex_t> *buffer, bounding_box_t *bbox, const grund_t *base) {
	const int x = base->get_pos().x;
	const int y = base->get_pos().y;
	const int z = base->get_hoehe();
	const int z_nw = base->get_hoehe(hang_t::corner_NW);
	const int z_ne = base->get_hoehe(hang_t::corner_NE);
	const int z_se = base->get_hoehe(hang_t::corner_SE);
	const int z_sw = base->get_hoehe(hang_t::corner_SW);

	bbox->include(x, y, z, max(max(max(z_nw, z_ne), z_se), z_sw));

	const texture_info_t *ti;
	if (base->get_typ() == grund_t::wasser) {
		image_id img = grund_besch_t::sea->get_bild(base->get_bild());
		ti = texture_atlas->get_texture_info(img);
	} else {
		ti = texture_atlas->get_texture_info(base->get_bild());
	}
	assert(ti);

	const float u_w = ti->u1;
	const float u_e = ti->u2;
	const float u_c = u_w + (u_e - u_w) / 2;

	const float v_se = calc_v1(ti, z, z_se, tile_height_step);
	const float v_nw = calc_v2(ti, z, z_nw, tile_height_step);
	const float v_ne = calc_v3(ti, z, z_ne, tile_height_step);
	const float v_sw = calc_v3(ti, z, z_sw, tile_height_step);

	if (z_sw == z_ne && z_nw != z_se) {
		buffer->push_back(vertex_t(x,   y,   z_nw, u_c, v_nw));
		buffer->push_back(vertex_t(x+1, y+1, z_se, u_c, v_se));
		buffer->push_back(vertex_t(x,   y+1, z_sw, u_w, v_sw));
		buffer->push_back(vertex_t(x,   y,   z_nw, u_c, v_nw));
		buffer->push_back(vertex_t(x+1, y,   z_ne, u_e, v_ne));
		buffer->push_back(vertex_t(x+1, y+1, z_se, u_c, v_se));
	}
	else {
		buffer->push_back(vertex_t(x,   y,   z_nw, u_c, v_nw));
		buffer->push_back(vertex_t(x+1, y,   z_ne, u_e, v_ne));
		buffer->push_back(vertex_t(x,   y+1, z_sw, u_w, v_sw));
		buffer->push_back(vertex_t(x+1, y,   z_ne, u_e, v_ne));
		buffer->push_back(vertex_t(x,   y+1, z_sw, u_w, v_sw));
		buffer->push_back(vertex_t(x+1, y+1, z_se, u_c, v_se));
	}
	size_t primitives = 2;

	const sint8 back_image = base->get_back_bild();
	if (back_image != 0) {
		const texture_info_t *ti1;
		const texture_info_t *ti2;

		if (abs(back_image) > 121) {
			// fence before a drop
			if (back_image < 0) {
				// behind a building
				ti1 = texture_atlas->get_texture_info(grund_besch_t::fences->get_bild(-back_image-122+3));
			}
			else {
				// on a normal tile
				ti1 = texture_atlas->get_texture_info(grund_besch_t::fences->get_bild(back_image-122));
			}
			ti2 = ti1;
		}
		else {
			// artificial slope
			const image_id back_image2 = (abs(back_image)/11)+11;
			const image_id back_image1 = abs(back_image)%11;


			if (back_image < 0) {
				ti1 = texture_atlas->get_texture_info(grund_besch_t::fundament->get_bild(back_image1));
				ti2 = texture_atlas->get_texture_info(grund_besch_t::fundament->get_bild(back_image2));
			}
			else {
				ti1 = texture_atlas->get_texture_info(grund_besch_t::slopes->get_bild(back_image1));
				ti2 = texture_atlas->get_texture_info(grund_besch_t::slopes->get_bild(back_image2));
			}
		}

		const float u_w = ti1->u1;
		const float u_e = ti2->u2;
		const float u_c1 = ti1->u1 + (ti1->u2 - ti1->u1) / 2.0f;
		const float u_c2 = ti2->u1 + (ti2->u2 - ti2->u1) / 2.0f;

		const float v_t1 = ti1->v1;
		const float v_t2 = ti2->v1;
		const float v_bs1 = ti1->v2 - ti1->v_per_pixel * (tile_h / 2);
		const float v_bb1 = ti1->v2 - ti1->v_per_pixel * tile_h;
		const float v_bs2 = ti2->v2 - ti2->v_per_pixel * (tile_h / 2);
		const float v_bb2 = ti2->v2 - ti2->v_per_pixel * tile_h;

		buffer->push_back(vertex_t(x,        y + 1.0f, z,                              u_w, v_bs1));
		buffer->push_back(vertex_t(x,        y + 1.0f, z + z_per_level - z_for_edge,   u_w, v_t1));
		buffer->push_back(vertex_t(x,        y,        z + z_per_level - 2*z_for_edge, u_c1, v_t1));
		buffer->push_back(vertex_t(x,        y + 1.0f, z,                              u_w, v_bs1));
		buffer->push_back(vertex_t(x,        y,        z + z_per_level - 2*z_for_edge, u_c1, v_t1));
		buffer->push_back(vertex_t(x,        y,        z,                              u_c1, v_bb1));

		buffer->push_back(vertex_t(x,        y,        z,                              u_c2, v_bb2));
		buffer->push_back(vertex_t(x,        y,        z + z_per_level - 2*z_for_edge, u_c2, v_t2));
		buffer->push_back(vertex_t(x + 1.0f, y,        z + z_per_level - z_for_edge,   u_e, v_t2));
		buffer->push_back(vertex_t(x,        y,        z,                              u_c2, v_bb2));
		buffer->push_back(vertex_t(x + 1.0f, y,        z + z_per_level - z_for_edge,   u_e, v_t2));
		buffer->push_back(vertex_t(x + 1.0f, y,        z,                              u_e, v_bs2));

		primitives += 4;
	}

	for (uint8 i = 0; i < base->get_top(); ++i) {
		const obj_t *thing = base->obj_bei(i);
		if (thing && !thing->is_moving() && !thing->is_non_static()) {
			primitives += add_object(buffer, bbox, base, thing);
		}
	}

	return primitives;
}

void static_geometry_t::dump(const std::vector<vertex_t> &buffer, const batch_info_t &batch_info, int i, int j) {
	char buf[512];
	snprintf(buf, 512, "cell_%d-%d.obj", i, j);
	FILE *f = fopen(buf, "wt");

	for (GLuint v = 0; v < (batch_info.ground_primitives * 3); ++v) {
		fprintf(f, "v %f %f %f\n", buffer[v].pos.x, buffer[v].pos.y, buffer[v].pos.z);
	}

	for (GLuint v = 0; v < (batch_info.ground_primitives * 3); ++v) {
		fprintf(f, "vt %f %f\n", buffer[v].uv.u, buffer[v].uv.v);
	}

	for (GLuint p = 0; p < batch_info.ground_primitives; ++p) {
		int a = p * 3 + 1;
		int b = a + 1;
		int c = a + 2;
		fprintf(f, "f %d/%d %d/%d %d/%d\n", a, a, b, b, c, c);
	}

	fclose(f);
}
