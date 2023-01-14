#version 450

layout(location = 0) out vec4 out_color;

layout(location = 0) in vec2 uv;

layout(binding = 0) uniform GlobalUBO {
	mat4 view;
	mat4 proj;
	mat4 inv_view;
	mat4 inv_proj;
} global;

layout(binding = 1) uniform LocalUBO {
	vec2 tile_map_unit;
	float exposure;
	float gamma;
} local;

layout (input_attachment_index = 0, set = 0, binding = 2) uniform subpassInput input_color_specular;
layout (input_attachment_index = 1, set = 0, binding = 3) uniform subpassInput input_light;


void main() {

    vec3 hdr_color = subpassLoad(input_color_specular).rgb * subpassLoad(input_light).rgb;
  
	// using the gamma correction / hdr from https://learnopengl.com/Advanced-Lighting/HDR
    // exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-hdr_color * local.exposure);
    // gamma correction 
    mapped = pow(mapped, vec3(1.0 / local.gamma));
  
    out_color = vec4(mapped, 1.0);
}