#version 450

// per vertex data
layout(location = 0) in vec3 aPos;

layout(location = 0) out vec3 sample_dir; // direction in which to sample the cubemap

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
	vec3 ambient_color;
} light;

void main() {

	sample_dir = aPos;
	mat4 rot = mat4(mat3(global.view));
	gl_Position = global.proj * rot * vec4(aPos, 1.0f);
	
}
