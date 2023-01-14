#version 450

layout(location = 0) out vec4 out_light_color_intensity;

layout(location = 0) in vec3 light_color;
layout(location = 1) in vec3 light_pos_rel_cam;
layout(location = 2) in float light_brightness;

layout (input_attachment_index = 0, set = 0, binding = 1) uniform subpassInput input_position;
layout (input_attachment_index = 1, set = 0, binding = 2) uniform subpassInput input_normal;
layout (input_attachment_index = 2, set = 0, binding = 3) uniform subpassInput input_color_specular;

void main() {

	vec3 frag_pos_rel_cam = subpassLoad(input_position).xyz;
	float intensity = light_brightness - length(frag_pos_rel_cam - light_pos_rel_cam);
	intensity = max(0.0f, intensity);

	out_light_color_intensity = vec4(light_color * intensity, 0.0f);
}