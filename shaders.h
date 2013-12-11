#ifndef SHADERS_H_INCLUDED
#define SHADERS_H_INCLUDED

#include <GL/gl.h>

extern void init_shaders();
extern void free_shaders();

extern void select_geometry_program(int texture_atlas_stage, int special_atlas_stage, int palette_stage);
extern GLuint get_geometry_program_player_attribute_index();

#endif // SHADERS_H_INCLUDED
