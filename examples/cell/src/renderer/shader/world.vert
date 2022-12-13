#version 450

// per vertex data
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aUv;
layout(location = 2) in vec3 aNormal;

// per cell data
layout(location = 0) in uint byte0;
layout(location = 1) in uint byte1;

layout(location = 0) out vec2 uv;
layout(location = 1) out vec3 normal;

layout(binding = 0) uniform UniformBufferObject {
	mat4 proj;
	mat4 view;
} ubo;

void main() {

	uv = aUv;
	normal = aNormal;

	//output the position of each vertex
	gl_Position = ubo.proj * ubo.view * vec4(aPos, 1.0f);
	
	// positive y is up, change my mind
	gl_Position.y = -gl_Position.y;
	
}
