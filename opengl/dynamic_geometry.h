#ifndef DYNAMIC_GEOMETRY_H
#define DYNAMIC_GEOMETRY_H

#include <vector>

#include "matrix.h"
#include "texture_atlas.h"
#include "geometry.h"

#include <GL/gl.h>

class dynamic_geometry_t : public geometry_manager_t {
public:
	dynamic_geometry_t(karte_t *world, const texture_atlas_t *texture_atlas);
	~dynamic_geometry_t();

	void display(const matrix_t &view_transform, int vw, int vh, const std::vector<bool> &visibility);

private:
	typedef std::vector<vertex_t> vertex_buffer;
	/**
	 * Used as a temporary working space.
	 */
	vertex_buffer work;

	GLuint vbos[2];
	uint8 vbo_index;

	int flush(int num_primitives);
};

#endif // DYNAMIC_GEOMETRY_H
