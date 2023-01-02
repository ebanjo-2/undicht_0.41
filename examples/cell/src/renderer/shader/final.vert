#version 450

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUv;

layout(location = 0) out vec2 uv;


layout(binding = 0) uniform GlobalUBO {
	mat4 proj;
	mat4 view;
	vec2 tile_map_unit;
} global;

void main() {
	
	uv = aUv;
	gl_Position = /*global.proj * global.view */ vec4(aPos, 1.0f, 1.0f);
	gl_Position.y = -gl_Position.y;
}
