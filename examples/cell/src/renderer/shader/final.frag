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
} local;


layout (binding = 2) uniform sampler2D tile_map;
layout (input_attachment_index = 0, set = 0, binding = 3) uniform subpassInput input_depth;
layout (input_attachment_index = 1, set = 0, binding = 4) uniform usubpassInput input_color;
layout (input_attachment_index = 2, set = 0, binding = 5) uniform subpassInput input_light;

vec3 getWorldPosition(float depth, vec2 screen_pos, mat4 inv_view, mat4 inv_proj);
vec2 getTexUV(vec3 pos, uint face, uvec2 material, vec2 uv_size);

void main() {
	
	// getting the data from the geometry buffer
	uvec4 geom_data = subpassLoad(input_color);
	uvec2 material = geom_data.xy;
	uint face_id  = geom_data.z;

	float depth = subpassLoad(input_depth).r;
	
	vec4 light = subpassLoad(input_light);

	// calculating the position of the fragment
	vec3 world_pos = getWorldPosition(depth, uv, global.inv_view, global.inv_proj);

	// calculating the uv on the tile map
	vec2 tile_map_uv = getTexUV(world_pos, face_id, material, local.tile_map_unit);

	out_color = texture(tile_map, tile_map_uv) + light;
	//out_color = vec4(world_pos, 1.0f) / 16;

}

vec3 getWorldPosition(float depth, vec2 screen_pos, mat4 inv_view, mat4 inv_proj) {
    // thanks for the math 
    // https://stackoverflow.com/questions/32227283/getting-world-position-from-depth-buffer-value

    vec4 clipSpacePosition = vec4(screen_pos * 2.0 - 1.0, depth, 1.0);
    vec4 viewSpacePosition = inv_proj * clipSpacePosition;

    // Perspective division
    viewSpacePosition /= viewSpacePosition.w;

    vec4 worldSpacePosition = inv_view * viewSpacePosition;
	
    return worldSpacePosition.xyz;
	//return viewSpacePosition;
}

vec2 getTexUV(vec3 pos, uint face, uvec2 material, vec2 uv_size) {

    vec3 rep_pos = fract(pos); // repeats from 0 to 1

	vec2 rep_uv = 
		float(bool(face & 0x03)) * rep_pos.zx + 
		float(bool(face & 0x0C)) * rep_pos.zy + 
		float(bool(face & 0x30)) * rep_pos.xy;

    vec2 tex_uv = material * uv_size;
    tex_uv += uv_size * rep_uv;
       
    return tex_uv;
}