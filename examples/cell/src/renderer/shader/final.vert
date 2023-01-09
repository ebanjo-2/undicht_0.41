#version 450

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUv;

layout(location = 0) out vec2 uv;



void main() {
	
	uv = aUv;
	gl_Position = vec4(aPos, 1.0f, 1.0f);
	gl_Position.y = -gl_Position.y;
}
