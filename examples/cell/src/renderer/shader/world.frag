#version 450

layout(location = 0) out vec4 out_color;

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 normal;

layout(binding = 0) uniform GlobalUBO {
	mat4 proj;
	mat4 view;
	vec2 tile_map_unit;
} global;


layout(binding = 2) uniform sampler2D tile_map;

void main() {

	out_color = texture(tile_map, uv);
	// out_color = vec4(normal, 1.0f);
}
