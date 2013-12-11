#ifndef GEOMETRY_H_INCLUDED
#define GEOMETRY_H_INCLUDED

#include <vector>

class karte_t;
class grund_t;
class brueckenboden_t;
class obj_t;
class vehikel_basis_t;
class texture_atlas_t;

struct pos_t {
	pos_t() {}
	pos_t(float x, float y, float z) : x(x), y(y), z(z) {}

	float x, y, z;
};

struct uv_t {
	uv_t() {}
	uv_t(float u, float v) : u(u), v(v) {}

	float u, v;
};

struct vertex_t {
	pos_t pos;
	uv_t uv;
	uint8 player;

	vertex_t() {}
	vertex_t(float x, float y, float z, float u = 0.0f, float v = 0.0f, uint8 player = 0) : pos(x, y, z), uv(u, v), player(player) {}
};

struct bounding_box_t {
	int min_x, min_y, min_z;
	int max_x, max_y, max_z;

	bounding_box_t();

	void include(int x, int y, int z_min, int z_max);
};

class geometry_manager_t {
protected:
	static const int BATCH_WH = 32;

	geometry_manager_t(karte_t *world, const texture_atlas_t *texture_atlas);

	karte_t *world;
	const texture_atlas_t *texture_atlas;

	const int tile_height_step;
	const int z_per_level;
	const int z_for_edge;
	const int tile_h;

	size_t add_object(std::vector<vertex_t> *buffer, bounding_box_t *bbox, const grund_t *base, const obj_t *object);
	size_t add_bridge(std::vector<vertex_t> *buffer, bounding_box_t *bbox, const brueckenboden_t *base, const obj_t *object);
	size_t add_vehicle(std::vector<vertex_t> *buffer, bounding_box_t *bbox, const grund_t *base, const vehikel_basis_t *object);

	void enable_array_pointers();
	void disable_array_pointers();
	void setup_array_pointers();

private:
	geometry_manager_t &operator =(const geometry_manager_t &other); // Forbidden
};

#endif // GEOMETRY_H_INCLUDED
