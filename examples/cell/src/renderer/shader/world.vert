#version 450

// per vertex data
layout(location = 0) in uvec3 aPos;
layout(location = 1) in uint aFaceID;

// per cell data
layout(location = 2) in uvec4 pos0;
layout(location = 3) in uvec4 pos1;

layout(location = 0) out flat uint face_id;
layout(location = 1) out flat vec2 material;
layout(location = 2) out vec3 pos_rel_cam;
layout(location = 3) out vec3 normal_rel_cam;
layout(location = 4) out vec2 cell_uv;

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

vec2 calcCellUv(vec3 vertex_pos, uint face);
vec3 calcCellNormal(uint face);

void main() {

	face_id = aFaceID;
	material = vec2(pos0.w, pos1.w) / 255.0f;

	vec3 vertex_pos = (1-aPos) * pos0.xyz + aPos * pos1.xyz;
	vec4 world_pos = vec4(vertex_pos + chunk.pos, 1.0f);

	// output to the fragment shader
	cell_uv = calcCellUv(vertex_pos, aFaceID);
	pos_rel_cam = (global.view * world_pos).xyz;
	normal_rel_cam = normalize(mat3(global.view) * calcCellNormal(aFaceID));

	gl_Position = global.proj * global.view * world_pos;
	gl_Position.y = -gl_Position.y; // positive y is up, change my mind
}

vec2 calcCellUv(vec3 vertex_pos, uint face) {

	vec2 cell_uv = 
		float(bool(face & 0x03)) * vertex_pos.zx + 
		float(bool(face & 0x0C)) * vertex_pos.zy + 
		float(bool(face & 0x30)) * vertex_pos.xy;

    return cell_uv;
}

vec3 calcCellNormal(uint face) {

	switch(face) {
	case 0x01:
		return vec3(0,1,0);
	case 0x02:
		return vec3(0,-1,0);
	case 0x04:
		return vec3(1,0,0);
	case 0x08:
		return vec3(-1,0,0);
	case 0x10:
		return vec3(0,0,1);
	case 0x20:
		return vec3(0,0,-1);
	default:
		return vec3(0,0,0);
	}

}
