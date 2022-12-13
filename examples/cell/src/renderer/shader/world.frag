#version 450

layout(location = 0) out vec4 outFragColor;

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 normal;


void main() {

	outFragColor = vec4(normal, 1.0f);
}
