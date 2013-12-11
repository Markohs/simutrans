#include <GL/glew.h>

static const char geometry_vertex_shader_src[] =
	"#version 110\n"
	"varying vec2 texture_coordinate;"
	"varying float player_v;"
	"attribute float player;"
	"void main() {"
		"texture_coordinate = vec2(gl_MultiTexCoord0);"
		"player_v = player;"
		"gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
	"}"
;

static const char geometry_fragment_shader_src[] =
	"#version 110\n"
	"varying vec2 texture_coordinate;"
	"varying float player_v;"
	"uniform sampler2D texture_atlas;"
	"uniform sampler2D special_atlas;"
	"uniform sampler2D palette;"
	"void main() {"
		"float special = texture2D(special_atlas, texture_coordinate).r;"
		"if (special < 1.0) {"
			"gl_FragColor = texture2D(palette, vec2(special, player_v));"
		"} else {"
			"gl_FragColor = texture2D(texture_atlas, texture_coordinate);"
		"}"
	"}"
;

GLuint geometry_program, geometry_vertex_shader, geometry_fragment_shader;
GLint geometry_program_texture_atlas_sampler, geometry_program_special_atlas_sampler, geometry_program_palette_sampler;
GLint geometry_program_player_attribute_index;

static void glShaderSourceWrapper(GLuint shader, const char *source) {
	glShaderSource(shader, 1, &source, 0);
}

void init_shaders() {
	geometry_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	geometry_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	geometry_program = glCreateProgram();

	glShaderSourceWrapper(geometry_vertex_shader, geometry_vertex_shader_src);
	glCompileShader(geometry_vertex_shader);

	glShaderSourceWrapper(geometry_fragment_shader, geometry_fragment_shader_src);
	glCompileShader(geometry_fragment_shader);

	glAttachShader(geometry_program, geometry_vertex_shader);
	glAttachShader(geometry_program, geometry_fragment_shader);
	glLinkProgram(geometry_program);

	geometry_program_texture_atlas_sampler = glGetUniformLocation(geometry_program, "texture_atlas");
	geometry_program_special_atlas_sampler = glGetUniformLocation(geometry_program, "special_atlas");
	geometry_program_palette_sampler = glGetUniformLocation(geometry_program, "palette");
	geometry_program_player_attribute_index = glGetAttribLocation(geometry_program, "player");
}

void free_shaders() {
	glDeleteProgram(geometry_program);
	glDeleteShader(geometry_fragment_shader);
	glDeleteShader(geometry_vertex_shader);
}

void select_geometry_program(int texture_atlas_stage, int special_atlas_stage, int palette_stage) {
	glUseProgram(geometry_program);
	glUniform1i(geometry_program_texture_atlas_sampler, texture_atlas_stage);
	glUniform1i(geometry_program_special_atlas_sampler, special_atlas_stage);
	glUniform1i(geometry_program_palette_sampler, palette_stage);
}

GLuint get_geometry_program_player_attribute_index() {
	return geometry_program_player_attribute_index;
}
