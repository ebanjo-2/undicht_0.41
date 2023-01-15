#version 450

// per vertex data
layout(location = 0) in vec3 aPos;

// per light data
layout(location = 1) in vec3 aLightPos;
layout(location = 2) in vec3 aColor;
layout(location = 3) in float aRange;
layout(location = 4) in float aLinFactor;
layout(location = 5) in float aQuaFactor;

layout(location = 0) out vec3 light_color;
layout(location = 1) out vec3 light_pos_rel_cam;
layout(location = 2) out float light_lin_factor;
layout(location = 3) out float light_qua_factor;

layout(binding = 0) uniform GlobalUBO {
	mat4 view;
	mat4 proj;
	mat4 inv_view;
	mat4 inv_proj;
	vec2 viewport;
	vec2 inv_viewport;
} global;

void main() {

	light_color = aColor;
	light_pos_rel_cam = (global.view * vec4(aLightPos, 1.0f)).xyz;
	light_lin_factor = aLinFactor;
	light_qua_factor = aQuaFactor;

	gl_Position = global.proj * global.view * vec4(aPos * aRange * 1.2f + aLightPos, 1.0f);
	
	// positive y is up, change my mind
	gl_Position.y = -gl_Position.y;

}
