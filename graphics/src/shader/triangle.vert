#version 450

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aUv;

layout(location = 0) out vec2 uv;

void main() {

	uv = aUv;

	//output the position of each vertex
	gl_Position = vec4(aPos, 1.0f);

}
