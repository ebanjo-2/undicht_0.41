#version 450

layout(location = 0) out vec4 out_material;
layout(location = 1) out vec4 out_normal;

layout(location = 0) in flat uint face_id;
layout(location = 1) in flat vec2 material;
layout(location = 2) in vec3 pos_rel_cam;
layout(location = 3) in vec3 normal_rel_cam;
layout(location = 4) in vec2 cell_uv;

layout(set = 0, binding = 0) uniform GlobalUBO {
	mat4 view;
	mat4 proj;
	mat4 inv_view;
	mat4 inv_proj;
	vec2 viewport;
	vec2 inv_viewport;
} global;

layout(set = 1, binding = 0) uniform LocalUBO {
	vec2 tile_map_unit;
} local;

layout(set = 2, binding = 0) uniform ChunkUBO {
	ivec3 pos;
} chunk;

void main() {

	out_material.xy = material;
	out_material.zw = fract(cell_uv);
	out_normal = vec4(normal_rel_cam, gl_FragCoord.z);
}
