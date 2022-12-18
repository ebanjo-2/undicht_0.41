#version 450

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aUv;

layout(location = 0) out vec2 uv;

layout(binding = 0) uniform UniformBufferObject {
	mat4 proj;
	mat4 view;
} ubo;


void main() {

	uv = aUv;

	//output the position of each vertex
	gl_Position = ubo.proj * ubo.view * vec4(aPos, 1.0f);
	
	// positive y is up, change my mind
	gl_Position.y = -gl_Position.y;	
}