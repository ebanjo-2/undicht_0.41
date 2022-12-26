#version 450

// per vertex data
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aUv;
layout(location = 2) in vec3 aNormal;

// per cell data
layout(location = 3) in uvec4 pos0;
layout(location = 4) in uvec4 pos1;

layout(location = 0) out vec2 uv;
layout(location = 1) out vec3 normal;

layout(binding = 0) uniform GlobalUBO {
	mat4 proj;
	mat4 view;
	vec2 tile_map_unit;
} global;

layout(binding = 1) uniform ChunkUBO {
	ivec3 pos;
} chunk;

void main() {



	uv = (vec2(pos0.w, pos1.w) + aUv) * global.tile_map_unit;
	// uv = aUv; // display full tile map on every cell
	normal = (aNormal + 1) / 2;

	// output the position of each vertex
	vec3 vertex_pos = (1-aPos) * pos0.xyz + aPos * pos1.xyz;
	gl_Position = global.proj * global.view * vec4(vertex_pos + chunk.pos, 1.0f);
	
	// positive y is up, change my mind
	gl_Position.y = -gl_Position.y;
	
}
