#version 450

layout(location = 0) out vec4 out_light_color_intensity;

layout(location = 0) in vec3 light_color;
layout(location = 1) in vec3 light_pos_rel_cam;
layout(location = 2) in float light_brightness;

layout(binding = 0) uniform GlobalUBO {
	mat4 view;
	mat4 proj;
	mat4 inv_view;
	mat4 inv_proj;
	vec2 viewport;
	vec2 inv_viewport;
} global;

layout (input_attachment_index = 0, set = 0, binding = 1) uniform subpassInput input_depth;
layout (input_attachment_index = 1, set = 0, binding = 2) uniform subpassInput input_material;
layout (input_attachment_index = 2, set = 0, binding = 3) uniform subpassInput input_normal;

vec3 calcFragPosRelCam(float depth, vec2 screen_pos, mat4 inv_proj);

void main() {

	// getting the position of the fragment from the depth buffer
	float depth = subpassLoad(input_depth).r;
	vec2 screen_pos = gl_FragCoord.xy * global.inv_viewport * 2.0 - 1.0;
	vec3 frag_pos_rel_cam = calcFragPosRelCam(depth, screen_pos, global.inv_proj);

	float intensity = light_brightness - length(frag_pos_rel_cam - light_pos_rel_cam);
	intensity = max(0.0f, intensity);

	out_light_color_intensity = vec4(light_color * intensity, 0.0f);
}

vec3 calcFragPosRelCam(float depth, vec2 screen_pos, mat4 inv_proj) {
    // thanks for the math 
    // https://stackoverflow.com/questions/32227283/getting-world-position-from-depth-buffer-value

    vec4 clipSpacePosition = vec4(screen_pos, depth, 1.0);
	clipSpacePosition.y = -clipSpacePosition.y;
    vec4 viewSpacePosition = inv_proj * clipSpacePosition;

    // Perspective division
    viewSpacePosition /= viewSpacePosition.w;

	return viewSpacePosition.xyz;
}