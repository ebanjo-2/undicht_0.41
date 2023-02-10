#version 450

layout(location = 0) out vec4 out_color;

layout(location = 0) in vec3 sample_dir;

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

layout(set = 1, binding = 0) uniform LocalUBO {
	vec2 tile_map_unit;
    ivec2 shadow_offset_texture_size;
} local;

layout (set = 1, binding = 1) uniform sampler2DArray tile_map;

layout (set = 1, input_attachment_index = 0, binding = 3) uniform subpassInput input_material;
layout (set = 1, input_attachment_index = 1, binding = 4) uniform subpassInput input_normal; // contains the normal data + depth

layout(set = 2, binding = 0) uniform LightUBO {
	vec3 ambient_color;
} light;

layout (set = 2, binding = 1) uniform samplerCube env_cube_map;

void main() {

    uvec2 material = uvec2(subpassLoad(input_material).xy * 255);

    if((material.x == 255) && (material.y == 255)) {
        out_color = texture(env_cube_map, sample_dir);
        // out_color = vec4(sample_dir, 0);
    } else {
        out_color = vec4(0,0,0,0);
    }

    
}