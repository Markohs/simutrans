#ifndef BATCH_GRID_H
#define BATCH_GRID_H

#include <vector>

#include "matrix.h"
#include "texture_atlas.h"
#include "geometry.h"

#include <GL/gl.h>

struct batch_info_t {
	batch_info_t() : ground_primitives(0), underground_primitives(0) {}

	bounding_box_t bbox;

	GLuint ground_primitives;
	GLuint underground_primitives;
};

class static_geometry_t : public geometry_manager_t {
public:
	static_geometry_t(karte_t *world, const texture_atlas_t *texture_atlas);
	~static_geometry_t();

	void reset();

	void set_dirty(int x, int y) {
		if (batches) {
			int i = x / BATCH_WH;
			int j = y / BATCH_WH;
			int index = j * grid_w + i;
			dirty[index] = true;
		}
	}

	void display(const matrix_t &view_transform, int vw, int vh);

	const std::vector<bool> &get_visibility() const { return visible; }

private:
	int grid_w, grid_h;

	GLuint *batches;
	batch_info_t *batch_infos;

	std::vector<bool> dirty;
	std::vector<bool> visible;

	/**
	 * Used as a temporary working space.
	 */
	std::vector<vertex_t> work;

	/**
	 * Used as a temporary working space for underground geometry.
	 */
	std::vector<vertex_t> work_underground;

	void regenerate();
	void do_dirty();
	void recreate_batch(int i, int j);
	size_t add_tile(std::vector<vertex_t> *buffer, bounding_box_t *bbox, const grund_t *base);

	void dump(const std::vector<vertex_t> &buffer, const batch_info_t &batch_info, int i, int j);
};

#endif // BATCH_GRID_H
