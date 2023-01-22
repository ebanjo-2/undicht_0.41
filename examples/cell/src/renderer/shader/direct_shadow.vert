#version 450

// per vertex data
layout(location = 0) in uvec3 aPos;
layout(location = 1) in uint aFaceID;

// per cell data
layout(location = 2) in uvec4 pos0;
layout(location = 3) in uvec4 pos1;

// global ubo unused

// local ubo unused
layout(set = 1, binding = 0) uniform LocalUBO {
	mat4 view;
	mat4 proj;
} local;

layout(set = 2, binding = 0) uniform ChunkUBO {
	ivec3 pos;
} chunk;


void main() {

	vec3 vertex_pos = (1-aPos) * pos0.xyz + aPos * pos1.xyz;
	vec4 world_pos = vec4(vertex_pos + chunk.pos, 1.0f);

	gl_Position = local.proj * local.view * world_pos;
	//gl_Position.y = -gl_Position.y; // positive y is up, change my mind
}