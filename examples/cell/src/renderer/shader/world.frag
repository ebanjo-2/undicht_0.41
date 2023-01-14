#version 450

layout(location = 0) out vec4 out_position;
layout(location = 1) out vec4 out_normal;
layout(location = 2) out vec4 out_color_specular;

layout(location = 0) in flat uint face_id;
layout(location = 1) in flat uvec2 material;
layout(location = 2) in vec3 pos_rel_cam;
layout(location = 3) in vec3 normal_rel_cam;
layout(location = 4) in vec2 cell_uv;

layout(binding = 0) uniform GlobalUBO {
	mat4 view;
	mat4 proj;
	mat4 inv_view;
	mat4 inv_proj;
} global;

layout(binding = 1) uniform LocalUBO {
	vec2 tile_map_unit;
} local;

layout(binding = 2) uniform ChunkUBO {
	ivec3 pos;
} chunk;

layout(binding = 3) uniform sampler2D tile_map;

void main() {

	vec2 uv = (material + fract(cell_uv)) * local.tile_map_unit;

	out_position = vec4(pos_rel_cam, 1.0f);
	out_normal = vec4(normal_rel_cam, 1.0f);
	out_color_specular = texture(tile_map, uv);

}
