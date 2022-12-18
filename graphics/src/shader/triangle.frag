#version 450

layout(location = 0) out vec4 outFragColor;

layout(location = 0) in vec2 uv;

layout(binding = 1) uniform sampler2D tex;

void main() {

	outFragColor = texture(tex, uv);
}

