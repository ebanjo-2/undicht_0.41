#version 450

// per vertex data
layout(location = 0) in uvec3 aPos;
layout(location = 1) in uint aFaceID;

// per cell data
layout(location = 2) in uvec4 pos0;
layout(location = 3) in uvec4 pos1;
layout(location = 4) in uint faces;

// global ubo (contains global shadow view + projection matrix)
layout(set = 0, binding = 0) uniform GlobalUBO {
	mat4 view;
	mat4 proj;
	mat4 inv_view;
	mat4 inv_proj;
	vec2 viewport;
	vec2 inv_viewport;
	mat4 shadow_view;
	mat4 shadow_proj;
} global;

// local ubo unused

// chunk ubo
layout(set = 2, binding = 0) uniform ChunkUBO {
	ivec3 pos;
} chunk;


void main() {

	vec3 vertex_pos = (1-aPos) * pos0.xyz + aPos * pos1.xyz;
	vec4 world_pos = vec4(vertex_pos + chunk.pos, 1.0f);
    bool draw_face = bool(aFaceID & faces); // 1 if the face should get drawn, 0 if not

	gl_Position = global.shadow_proj * global.shadow_view * world_pos * float(draw_face);
}