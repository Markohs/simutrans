#include <GL/glew.h>

#include "../simworld.h"
#include "../player/simplay.h"
#include "../boden/brueckenboden.h"
#include "../vehicle/simvehikel.h"
#include "../dataobj/environment.h"

#include "texture_atlas.h"
#include "geometry.h"
#include "shaders.h"

geometry_manager_t::geometry_manager_t(karte_t *world, const texture_atlas_t *texture_atlas) :
	world(world),
	texture_atlas(texture_atlas),
	tile_height_step(tile_raster_scale_y(TILE_HEIGHT_STEP, get_base_tile_raster_width())),
	z_per_level(get_base_tile_raster_width() / tile_height_step),
	z_for_edge(get_base_tile_raster_width() / tile_height_step / 4),
	tile_h(get_base_tile_raster_width() / 2) {
}

void geometry_manager_t::enable_array_pointers() {
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableVertexAttribArray(get_geometry_program_player_attribute_index());
}

void geometry_manager_t::disable_array_pointers() {
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableVertexAttribArray(get_geometry_program_player_attribute_index());
}

void geometry_manager_t::setup_array_pointers() {
	static const uint8 *base_ptr = 0;
	glVertexPointer(3, GL_FLOAT, sizeof(vertex_t), base_ptr + offsetof(vertex_t, pos));
	glTexCoordPointer(2, GL_FLOAT, sizeof(vertex_t), base_ptr + offsetof(vertex_t, uv));
	glVertexAttribPointer(get_geometry_program_player_attribute_index(), 1, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(vertex_t), base_ptr + offsetof(vertex_t, player));
}

size_t geometry_manager_t::add_object(std::vector<vertex_t> *buffer, bounding_box_t *bbox, const grund_t *base, const obj_t *object) {
	if (vehikel_basis_t const* vehicle = obj_cast<vehikel_basis_t>(object)) {
		return add_vehicle(buffer, bbox, base, vehicle);
	}
	if (base->get_typ() == grund_t::brueckenboden) {
		return add_bridge(buffer, bbox, static_cast<const brueckenboden_t *>(base), object);
	}

	const int tx = object->get_pos().x;
	const int ty = object->get_pos().y;
	const int tz = object->get_pos().z;
	const uint8 player = object->get_besitzer() ? object->get_besitzer()->get_player_nr() : PLAYER_UNOWNED;

	size_t primitives = 0;

	int ox = 0, oy = 0;

	ox += tile_raster_scale_x(object->get_xoff(), get_base_tile_raster_width());
	oy += tile_raster_scale_x(object->get_yoff(), get_base_tile_raster_width());

	const float fx = float(ox + 2 * oy) / get_base_tile_raster_width();
	const float fy = float(-ox + 2 * oy) / get_base_tile_raster_width();

	const float x = tx + fx;
	const float y = ty + fy;
	float z = tz;
	uint8 p = player << 4;

	// Do front image first, since z is still unmodified. Z-buffering also prefers front-to-back.
	if (object->get_after_bild() != IMG_LEER) {
		const texture_info_t *ti = texture_atlas->get_texture_info(object->get_after_bild());

		const float u_w = ti->u1;
		const float u_e = ti->u2;
		const float u_c = ti->u1 + (ti->u2 - ti->u1) / 2.0f;

		const float v_b = ti->v2;
		const float v_t = ti->v1;

		buffer->push_back(vertex_t(x,        y + 1.0f, z - z_for_edge,               u_w, v_b, p));
		buffer->push_back(vertex_t(x,        y + 1.0f, z + z_per_level - z_for_edge, u_w, v_t, p));
		buffer->push_back(vertex_t(x + 1.0f, y + 1.0f, z + z_per_level,              u_c, v_t, p));
		buffer->push_back(vertex_t(x,        y + 1.0f, z - z_for_edge,               u_w, v_b, p));
		buffer->push_back(vertex_t(x + 1.0f, y + 1.0f, z + z_per_level,              u_c, v_t, p));
		buffer->push_back(vertex_t(x + 1.0f, y + 1.0f, z,                            u_c, v_b, p));

		buffer->push_back(vertex_t(x + 1.0f, y + 1.0f, z,                            u_c, v_b, p));
		buffer->push_back(vertex_t(x + 1.0f, y + 1.0f, z + z_per_level,              u_c, v_t, p));
		buffer->push_back(vertex_t(x + 1.0f, y,        z + z_per_level - z_for_edge, u_e, v_t, p));
		buffer->push_back(vertex_t(x + 1.0f, y + 1.0f, z,                            u_c, v_b, p));
		buffer->push_back(vertex_t(x + 1.0f, y,        z + z_per_level - z_for_edge, u_e, v_t, p));
		buffer->push_back(vertex_t(x + 1.0f, y,        z - z_for_edge,               u_e, v_b, p));

		primitives += 4;
	}

	int i = 0;
	image_id img = object->get_bild();
	while (img != IMG_LEER) {
		const texture_info_t *ti = texture_atlas->get_texture_info(img);

		const float u_w = ti->u1;
		const float u_e = ti->u2;
		const float u_c = ti->u1 + (ti->u2 - ti->u1) / 2.0f;

		if (i == 0) {
			const float z_nw = std::max(float(base->get_hoehe(hang_t::corner_NW)), z);
			const float z_ne = std::max(float(base->get_hoehe(hang_t::corner_NE)), z);
			const float z_se = std::max(float(base->get_hoehe(hang_t::corner_SE)), z);
			const float z_sw = std::max(float(base->get_hoehe(hang_t::corner_SW)), z);

			const float v_t = ti->v1;
			const float v_bf = ti->v2 - ti->v_per_pixel * (z_se - z) * tile_height_step;
			const float v_bl = ti->v2 - ti->v_per_pixel * (tile_h / 2) - ti->v_per_pixel * (z_sw - z) * tile_height_step;
			const float v_br = ti->v2 - ti->v_per_pixel * (tile_h / 2) - ti->v_per_pixel * (z_ne - z) * tile_height_step;
			const float v_bb = ti->v2 - ti->v_per_pixel * tile_h - ti->v_per_pixel * (z_nw - z) * tile_height_step;

			buffer->push_back(vertex_t(x,        y + 1.0f, z_sw,                           u_w, v_bl, p));
			buffer->push_back(vertex_t(x,        y,        z_nw,                           u_c, v_bb, p));
			buffer->push_back(vertex_t(x + 1.0f, y,        z_ne,                           u_e, v_br, p));
			buffer->push_back(vertex_t(x,        y + 1.0f, z_sw,                           u_w, v_bl, p));
			buffer->push_back(vertex_t(x + 1.0f, y,        z_ne,                           u_e, v_br, p));
			buffer->push_back(vertex_t(x + 1.0f, y + 1.0f, z_se,                           u_c, v_bf, p));

			buffer->push_back(vertex_t(x,        y + 1.0f, z_sw,                           u_w, v_bl, p));
			buffer->push_back(vertex_t(x,        y + 1.0f, z + z_per_level - z_for_edge,   u_w, v_t,  p));
			buffer->push_back(vertex_t(x,        y,        z + z_per_level - 2*z_for_edge, u_c, v_t,  p));
			buffer->push_back(vertex_t(x,        y + 1.0f, z_sw,                           u_w, v_bl, p));
			buffer->push_back(vertex_t(x,        y,        z + z_per_level - 2*z_for_edge, u_c, v_t,  p));
			buffer->push_back(vertex_t(x,        y,        z_nw,                           u_c, v_bb, p));

			buffer->push_back(vertex_t(x,        y,        z_nw,                           u_c, v_bb, p));
			buffer->push_back(vertex_t(x,        y,        z + z_per_level - 2*z_for_edge, u_c, v_t,  p));
			buffer->push_back(vertex_t(x + 1.0f, y,        z + z_per_level - z_for_edge,   u_e, v_t,  p));
			buffer->push_back(vertex_t(x,        y,        z_nw,                           u_c, v_bb, p));
			buffer->push_back(vertex_t(x + 1.0f, y,        z + z_per_level - z_for_edge,   u_e, v_t,  p));
			buffer->push_back(vertex_t(x + 1.0f, y,        z_ne,                           u_e, v_br, p));

			primitives += 6;
		}
		else {
			const float v_t = ti->v1;
			const float v_b = ti->v2;

			buffer->push_back(vertex_t(x,        y + 1.0f, z - z_for_edge,                 u_w, v_b, p));
			buffer->push_back(vertex_t(x,        y + 1.0f, z + z_per_level - z_for_edge,   u_w, v_t, p));
			buffer->push_back(vertex_t(x,        y,        z + z_per_level - 2*z_for_edge, u_c, v_t, p));
			buffer->push_back(vertex_t(x,        y + 1.0f, z - z_for_edge,                 u_w, v_b, p));
			buffer->push_back(vertex_t(x,        y,        z + z_per_level - 2*z_for_edge, u_c, v_t, p));
			buffer->push_back(vertex_t(x,        y,        z - 2*z_for_edge,               u_c, v_b, p));

			buffer->push_back(vertex_t(x,        y,        z - 2*z_for_edge,               u_c, v_b, p));
			buffer->push_back(vertex_t(x,        y,        z + z_per_level - 2*z_for_edge, u_c, v_t, p));
			buffer->push_back(vertex_t(x + 1.0f, y,        z + z_per_level - z_for_edge,   u_e, v_t, p));
			buffer->push_back(vertex_t(x,        y,        z - 2*z_for_edge,               u_c, v_b, p));
			buffer->push_back(vertex_t(x + 1.0f, y,        z + z_per_level - z_for_edge,   u_e, v_t, p));
			buffer->push_back(vertex_t(x + 1.0f, y,        z - z_for_edge,                 u_e, v_b, p));

			primitives += 4;
		}

		if (bbox) bbox->include(x, y, z, z + z_per_level);

		z += z_per_level;
		img = object->get_bild(++i);
	}

	return primitives;
}

/**
 * Bridges have a y-offset. Must transform this to a z-offset.
 */
size_t geometry_manager_t::add_bridge(std::vector<vertex_t> *buffer, bounding_box_t *bbox, const brueckenboden_t *base, const obj_t *object) {
	const int tx = object->get_pos().x;
	const int ty = object->get_pos().y;
	const int tz = object->get_pos().z;
	const uint8 player = object->get_besitzer() ? object->get_besitzer()->get_player_nr() : PLAYER_UNOWNED;

	size_t primitives = 0;

	const float x = tx;
	const float y = ty;
	float z = tz;
	if (base->get_weg_hang() == 0 && base->get_grund_hang() != hang_t::flach) {
		z += 1.0f;
	}
	uint8 p = player << 4;

	// Do front image first, since z is still unmodified. Z-buffering also prefers front-to-back.
	if (object->get_after_bild() != IMG_LEER) {
		const texture_info_t *ti = texture_atlas->get_texture_info(object->get_after_bild());

		const float u_w = ti->u1;
		const float u_e = ti->u2;
		const float u_c = ti->u1 + (ti->u2 - ti->u1) / 2.0f;

		const float v_b = ti->v2;
		const float v_t = ti->v1;

		buffer->push_back(vertex_t(x,        y + 1.0f, z - z_for_edge,               u_w, v_b, p));
		buffer->push_back(vertex_t(x,        y + 1.0f, z + z_per_level - z_for_edge, u_w, v_t, p));
		buffer->push_back(vertex_t(x + 1.0f, y + 1.0f, z + z_per_level,              u_c, v_t, p));
		buffer->push_back(vertex_t(x,        y + 1.0f, z - z_for_edge,               u_w, v_b, p));
		buffer->push_back(vertex_t(x + 1.0f, y + 1.0f, z + z_per_level,              u_c, v_t, p));
		buffer->push_back(vertex_t(x + 1.0f, y + 1.0f, z,                            u_c, v_b, p));

		buffer->push_back(vertex_t(x + 1.0f, y + 1.0f, z,                            u_c, v_b, p));
		buffer->push_back(vertex_t(x + 1.0f, y + 1.0f, z + z_per_level,              u_c, v_t, p));
		buffer->push_back(vertex_t(x + 1.0f, y,        z + z_per_level - z_for_edge, u_e, v_t, p));
		buffer->push_back(vertex_t(x + 1.0f, y + 1.0f, z,                            u_c, v_b, p));
		buffer->push_back(vertex_t(x + 1.0f, y,        z + z_per_level - z_for_edge, u_e, v_t, p));
		buffer->push_back(vertex_t(x + 1.0f, y,        z - z_for_edge,               u_e, v_b, p));

		primitives += 4;
	}

	int i = 0;
	image_id img = object->get_bild();
	while (img != IMG_LEER) {
		const texture_info_t *ti = texture_atlas->get_texture_info(img);

		const float u_w = ti->u1;
		const float u_e = ti->u2;
		const float u_c = ti->u1 + (ti->u2 - ti->u1) / 2.0f;

		if (i == 0) {
			const float v_t = ti->v1;
			const float v_bf = ti->v2;
			const float v_bs = ti->v2 - ti->v_per_pixel * (tile_h / 2);
			const float v_bb = ti->v2 - ti->v_per_pixel * tile_h;

			buffer->push_back(vertex_t(x,        y + 1.0f, z,                              u_w, v_bs, p));
			buffer->push_back(vertex_t(x,        y,        z,                              u_c, v_bb, p));
			buffer->push_back(vertex_t(x + 1.0f, y,        z,                              u_e, v_bs, p));
			buffer->push_back(vertex_t(x,        y + 1.0f, z,                              u_w, v_bs, p));
			buffer->push_back(vertex_t(x + 1.0f, y,        z,                              u_e, v_bs, p));
			buffer->push_back(vertex_t(x + 1.0f, y + 1.0f, z,                              u_c, v_bf, p));

			buffer->push_back(vertex_t(x,        y + 1.0f, z,                              u_w, v_bs, p));
			buffer->push_back(vertex_t(x,        y + 1.0f, z + z_per_level - z_for_edge,   u_w, v_t,  p));
			buffer->push_back(vertex_t(x,        y,        z + z_per_level - 2*z_for_edge, u_c, v_t,  p));
			buffer->push_back(vertex_t(x,        y + 1.0f, z,                              u_w, v_bs, p));
			buffer->push_back(vertex_t(x,        y,        z + z_per_level - 2*z_for_edge, u_c, v_t,  p));
			buffer->push_back(vertex_t(x,        y,        z,                              u_c, v_bb, p));

			buffer->push_back(vertex_t(x,        y,        z,                              u_c, v_bb, p));
			buffer->push_back(vertex_t(x,        y,        z + z_per_level - 2*z_for_edge, u_c, v_t,  p));
			buffer->push_back(vertex_t(x + 1.0f, y,        z + z_per_level - z_for_edge,   u_e, v_t,  p));
			buffer->push_back(vertex_t(x,        y,        z,                              u_c, v_bb, p));
			buffer->push_back(vertex_t(x + 1.0f, y,        z + z_per_level - z_for_edge,   u_e, v_t,  p));
			buffer->push_back(vertex_t(x + 1.0f, y,        z,                              u_e, v_bs, p));

			buffer->push_back(vertex_t(x,        y + 1.0f, z - z_for_edge,                 u_w, v_bf, p));
			buffer->push_back(vertex_t(x,        y + 1.0f, z,                              u_w, v_bs, p));
			buffer->push_back(vertex_t(x + 1.0f, y + 1.0f, z,                              u_c, v_bf, p));
			buffer->push_back(vertex_t(x + 1.0f, y + 1.0f, z,                              u_c, v_bf, p));
			buffer->push_back(vertex_t(x + 1.0f, y,        z,                              u_e, v_bs, p));
			buffer->push_back(vertex_t(x + 1.0f, y,        z - z_for_edge,                 u_e, v_bf, p));

			primitives += 8;
		}
		else {
			const float v_t = ti->v1;
			const float v_b = ti->v2;

			buffer->push_back(vertex_t(x,        y + 1.0f, z - z_for_edge,                 u_w, v_b, p));
			buffer->push_back(vertex_t(x,        y + 1.0f, z + z_per_level - z_for_edge,   u_w, v_t, p));
			buffer->push_back(vertex_t(x,        y,        z + z_per_level - 2*z_for_edge, u_c, v_t, p));
			buffer->push_back(vertex_t(x,        y + 1.0f, z - z_for_edge,                 u_w, v_b, p));
			buffer->push_back(vertex_t(x,        y,        z + z_per_level - 2*z_for_edge, u_c, v_t, p));
			buffer->push_back(vertex_t(x,        y,        z - 2*z_for_edge,               u_c, v_b, p));

			buffer->push_back(vertex_t(x,        y,        z - 2*z_for_edge,               u_c, v_b, p));
			buffer->push_back(vertex_t(x,        y,        z + z_per_level - 2*z_for_edge, u_c, v_t, p));
			buffer->push_back(vertex_t(x + 1.0f, y,        z + z_per_level - z_for_edge,   u_e, v_t, p));
			buffer->push_back(vertex_t(x,        y,        z - 2*z_for_edge,               u_c, v_b, p));
			buffer->push_back(vertex_t(x + 1.0f, y,        z + z_per_level - z_for_edge,   u_e, v_t, p));
			buffer->push_back(vertex_t(x + 1.0f, y,        z - z_for_edge,                 u_e, v_b, p));

			primitives += 4;
		}

		if (bbox) bbox->include(x, y, z, z + z_per_level);

		z += z_per_level;
		img = object->get_bild(++i);
	}

	return primitives;
}

static float add_hang(float base_z, hang_t::typ hang, hang_t::typ corner) {
	return hang & corner ? base_z + 1.0f : base_z;
}

/**
 * Vehicles have special offsets and their own issues with geometry.
 */
size_t geometry_manager_t::add_vehicle(std::vector<vertex_t> *buffer, bounding_box_t *bbox, const grund_t *base, const vehikel_basis_t *object) {
	const int tx = object->get_pos().x;
	const int ty = object->get_pos().y;
	const int tz = object->get_pos().z;
	const uint8 player = object->get_besitzer() ? object->get_besitzer()->get_player_nr() : PLAYER_UNOWNED;

	size_t primitives = 0;

	int ox, oy;
	float oz;

	// vehicles needs finer steps to appear smoother
	sint32 display_steps = (uint32)object->get_steps()*(uint16)get_base_tile_raster_width();
	if(object->get_dx()*object->get_dy()) {
		display_steps &= 0xFFFFFC00;
	}
	else {
		display_steps = (display_steps*vehikel_basis_t::get_diagonal_multiplier())>>10;
	}
	ox = (display_steps*object->get_dx()) >> 10;
	oy = ((display_steps*object->get_dy()) >> 10);
	oz = -object->get_hoff() / 16.0f;

	// TODO: Drive on left

	ox += tile_raster_scale_x(object->get_xoff(), get_base_tile_raster_width());
	oy += tile_raster_scale_x(object->get_yoff(), get_base_tile_raster_width());

	const float fx = float(ox + 2 * oy) / get_base_tile_raster_width();
	const float fy = float(-ox + 2 * oy) / get_base_tile_raster_width();

	const float x = tx + fx;
	const float y = ty + fy;
	float z = tz + oz;
	uint8 p = player << 4;

	// Do front image first, since z is still unmodified. Z-buffering also prefers front-to-back.
	if (object->get_after_bild() != IMG_LEER) {
		const texture_info_t *ti = texture_atlas->get_texture_info(object->get_after_bild());

		const float u_w = ti->u1;
		const float u_e = ti->u2;
		const float u_c = ti->u1 + (ti->u2 - ti->u1) / 2.0f;

		const float v_b = ti->v2;
		const float v_t = ti->v1;

		buffer->push_back(vertex_t(x,        y + 1.0f, z - z_for_edge,               u_w, v_b, p));
		buffer->push_back(vertex_t(x,        y + 1.0f, z + z_per_level - z_for_edge, u_w, v_t, p));
		buffer->push_back(vertex_t(x + 1.0f, y + 1.0f, z + z_per_level,              u_c, v_t, p));
		buffer->push_back(vertex_t(x,        y + 1.0f, z - z_for_edge,               u_w, v_b, p));
		buffer->push_back(vertex_t(x + 1.0f, y + 1.0f, z + z_per_level,              u_c, v_t, p));
		buffer->push_back(vertex_t(x + 1.0f, y + 1.0f, z,                            u_c, v_b, p));

		buffer->push_back(vertex_t(x + 1.0f, y + 1.0f, z,                            u_c, v_b, p));
		buffer->push_back(vertex_t(x + 1.0f, y + 1.0f, z + z_per_level,              u_c, v_t, p));
		buffer->push_back(vertex_t(x + 1.0f, y,        z + z_per_level - z_for_edge, u_e, v_t, p));
		buffer->push_back(vertex_t(x + 1.0f, y + 1.0f, z,                            u_c, v_b, p));
		buffer->push_back(vertex_t(x + 1.0f, y,        z + z_per_level - z_for_edge, u_e, v_t, p));
		buffer->push_back(vertex_t(x + 1.0f, y,        z - z_for_edge,               u_e, v_b, p));

		primitives += 4;
	}

	int i = 0;
	image_id img = object->get_bild();
	while (img != IMG_LEER) {
		const texture_info_t *ti = texture_atlas->get_texture_info(img);

		const float u_w = ti->u1;
		const float u_e = ti->u2;
		const float u_c = ti->u1 + (ti->u2 - ti->u1) / 2.0f;

		if (i == 0) {
			const hang_t::typ hang = base->get_weg_hang();
			const float z_nw = add_hang(z, hang, hang_t::corner_NW);
			const float z_ne = add_hang(z, hang, hang_t::corner_NE);
			const float z_se = add_hang(z, hang, hang_t::corner_SE);
			const float z_sw = add_hang(z, hang, hang_t::corner_SW);

			const float v_t = ti->v1;
			const float v_bf = ti->v2 - ti->v_per_pixel * (z_se - z) * tile_height_step;
			const float v_bl = ti->v2 - ti->v_per_pixel * (tile_h / 2) - ti->v_per_pixel * (z_sw - z) * tile_height_step;
			const float v_br = ti->v2 - ti->v_per_pixel * (tile_h / 2) - ti->v_per_pixel * (z_ne - z) * tile_height_step;
			const float v_bb = ti->v2 - ti->v_per_pixel * tile_h - ti->v_per_pixel * (z_nw - z) * tile_height_step;

			buffer->push_back(vertex_t(x,        y + 1.0f, z_sw,                           u_w, v_bl, p));
			buffer->push_back(vertex_t(x,        y,        z_nw,                           u_c, v_bb, p));
			buffer->push_back(vertex_t(x + 1.0f, y,        z_ne,                           u_e, v_br, p));
			buffer->push_back(vertex_t(x,        y + 1.0f, z_sw,                           u_w, v_bl, p));
			buffer->push_back(vertex_t(x + 1.0f, y,        z_ne,                           u_e, v_br, p));
			buffer->push_back(vertex_t(x + 1.0f, y + 1.0f, z_se,                           u_c, v_bf, p));

			buffer->push_back(vertex_t(x,        y + 1.0f, z_sw,                           u_w, v_bl, p));
			buffer->push_back(vertex_t(x,        y + 1.0f, z + z_per_level - z_for_edge,   u_w, v_t,  p));
			buffer->push_back(vertex_t(x,        y,        z + z_per_level - 2*z_for_edge, u_c, v_t,  p));
			buffer->push_back(vertex_t(x,        y + 1.0f, z_sw,                           u_w, v_bl, p));
			buffer->push_back(vertex_t(x,        y,        z + z_per_level - 2*z_for_edge, u_c, v_t,  p));
			buffer->push_back(vertex_t(x,        y,        z_nw,                           u_c, v_bb, p));

			buffer->push_back(vertex_t(x,        y,        z_nw,                           u_c, v_bb, p));
			buffer->push_back(vertex_t(x,        y,        z + z_per_level - 2*z_for_edge, u_c, v_t,  p));
			buffer->push_back(vertex_t(x + 1.0f, y,        z + z_per_level - z_for_edge,   u_e, v_t,  p));
			buffer->push_back(vertex_t(x,        y,        z_nw,                           u_c, v_bb, p));
			buffer->push_back(vertex_t(x + 1.0f, y,        z + z_per_level - z_for_edge,   u_e, v_t,  p));
			buffer->push_back(vertex_t(x + 1.0f, y,        z_ne,                           u_e, v_br, p));

			primitives += 6;
		}
		else {
			const float v_t = ti->v1;
			const float v_b = ti->v2;

			buffer->push_back(vertex_t(x,        y + 1.0f, z - z_for_edge,                 u_w, v_b, p));
			buffer->push_back(vertex_t(x,        y + 1.0f, z + z_per_level - z_for_edge,   u_w, v_t, p));
			buffer->push_back(vertex_t(x,        y,        z + z_per_level - 2*z_for_edge, u_c, v_t, p));
			buffer->push_back(vertex_t(x,        y + 1.0f, z - z_for_edge,                 u_w, v_b, p));
			buffer->push_back(vertex_t(x,        y,        z + z_per_level - 2*z_for_edge, u_c, v_t, p));
			buffer->push_back(vertex_t(x,        y,        z - 2*z_for_edge,               u_c, v_b, p));

			buffer->push_back(vertex_t(x,        y,        z - 2*z_for_edge,               u_c, v_b, p));
			buffer->push_back(vertex_t(x,        y,        z + z_per_level - 2*z_for_edge, u_c, v_t, p));
			buffer->push_back(vertex_t(x + 1.0f, y,        z + z_per_level - z_for_edge,   u_e, v_t, p));
			buffer->push_back(vertex_t(x,        y,        z - 2*z_for_edge,               u_c, v_b, p));
			buffer->push_back(vertex_t(x + 1.0f, y,        z + z_per_level - z_for_edge,   u_e, v_t, p));
			buffer->push_back(vertex_t(x + 1.0f, y,        z - z_for_edge,                 u_e, v_b, p));

			primitives += 4;
		}

		if (bbox) bbox->include(x, y, z, z + z_per_level);

		z += z_per_level;
		img = static_cast<const obj_t *>(object)->get_bild(++i);
	}

	return primitives;
}
