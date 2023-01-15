#version 450

layout(location = 0) out vec4 out_color;

layout(location = 0) in vec2 uv;

layout(binding = 0) uniform GlobalUBO {
	mat4 view;
	mat4 proj;
	mat4 inv_view;
	mat4 inv_proj;
	vec2 viewport;
	vec2 inv_viewport;
} global;

layout(binding = 1) uniform LocalUBO {
	float exposure;
	float gamma;
} local;

layout (input_attachment_index = 1, set = 0, binding = 2) uniform subpassInput input_light;


void main() {

	/*uvec2 material = uvec2(subpassLoad(input_material).xy * 255);
	vec2 cell_uv = subpassLoad(input_material).zw;
	vec2 tile_map_uv = (material + cell_uv * 0.99) * local.tile_map_unit;

	vec3 material_albedo = texture(tile_map, vec3(tile_map_uv, 1)).rgb;*/

	// using the gamma correction / exposure tone mapping from https://learnopengl.com/Advanced-Lighting/HDR
    vec3 hdr_color = subpassLoad(input_light).rgb;
    vec3 mapped = vec3(1.0) - exp(-hdr_color * local.exposure); // exposure tone mapping
    mapped = pow(mapped, vec3(1.0 / local.gamma)); // gamma correction 
  
    out_color = vec4(mapped, 1.0);
}