#version 450

// per vertex data
layout(location = 0) in uvec3 aPos;
layout(location = 1) in uint aFaceID;

// per cell data
layout(location = 2) in uvec4 pos0;
layout(location = 3) in uvec4 pos1;

layout(location = 0) out flat uint face_id;
layout(location = 1) out flat uvec2 material;

layout(binding = 0) uniform GlobalUBO {
	mat4 proj;
	mat4 view;
	vec2 tile_map_unit;
} global;

layout(binding = 1) uniform ChunkUBO {
	ivec3 pos;
} chunk;

void main() {

	face_id = aFaceID;
	material = uvec2(pos0.w, pos1.w);

	// output the position of each vertex
	vec3 vertex_pos = (1-aPos) * pos0.xyz + aPos * pos1.xyz;
	gl_Position = global.proj * global.view * vec4(vertex_pos + chunk.pos, 1.0f);
	
	// positive y is up, change my mind
	gl_Position.y = -gl_Position.y;
}
