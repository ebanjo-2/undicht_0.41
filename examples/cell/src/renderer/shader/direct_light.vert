#version 450

// per vertex data
layout(location = 0) in vec3 aPos;

layout(location = 0) out vec3 light_dir_rel_cam;

layout(set = 0, binding = 0) uniform GlobalUBO {
	mat4 view;
	mat4 proj;
	mat4 inv_view;
	mat4 inv_proj;
	vec2 viewport;
	vec2 inv_viewport;
} global;

// inputs specific to this type of light renderer
layout(set = 2, binding = 0) uniform LightUBO {
	vec3 color;
    vec3 direction;
} light;

void main() {

	light_dir_rel_cam = normalize(mat3(global.view) * light.direction);

	gl_Position = vec4(aPos, 1.0f);
}
