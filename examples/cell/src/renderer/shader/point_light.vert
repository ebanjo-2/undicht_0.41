#version 450

// per vertex data
layout(location = 0) in vec3 aPos;

// per light data
layout(location = 1) in vec3 aLightPos;
layout(location = 2) in vec3 aColor;
layout(location = 3) in float aRange;

layout(location = 0) out vec3 light_color;
layout(location = 1) out vec3 light_pos_rel_cam;

layout(set = 0, binding = 0) uniform GlobalUBO {
	mat4 view;
	mat4 proj;
	mat4 inv_view;
	mat4 inv_proj;
	vec2 viewport;
	vec2 inv_viewport;
	mat4 shadow_view;
	mat4 shadow_proj;
} global;

void main() {

	light_color = aColor;
	light_pos_rel_cam = (global.view * vec4(aLightPos, 1.0f)).xyz;

	gl_Position = global.proj * global.view * vec4(aPos * aRange * 1.2f + aLightPos, 1.0f);
}
