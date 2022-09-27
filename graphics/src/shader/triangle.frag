#version 450

layout(location = 0) out vec4 outFragColor;

layout(location = 0) in vec2 uv;

void main() {

	outFragColor = vec4(uv, 0.0f,1.0f);
}

