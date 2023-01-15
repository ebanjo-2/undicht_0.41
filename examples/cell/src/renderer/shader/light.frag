#version 450

layout(location = 0) out vec4 out_color;

layout(location = 0) in vec3 light_color;
layout(location = 1) in vec3 light_pos_rel_cam;
layout(location = 2) in float light_lin_factor;
layout(location = 3) in float light_qua_factor;

layout(binding = 0) uniform GlobalUBO {
	mat4 view;
	mat4 proj;
	mat4 inv_view;
	mat4 inv_proj;
	vec2 viewport;
	vec2 inv_viewport;
} global;

layout(binding = 1) uniform LocalUBO {
	vec2 tile_map_unit;
} local;

layout (binding = 2) uniform sampler2DArray tile_map;

layout (input_attachment_index = 0, set = 0, binding = 3) uniform subpassInput input_depth;
layout (input_attachment_index = 1, set = 0, binding = 4) uniform subpassInput input_material;
layout (input_attachment_index = 2, set = 0, binding = 5) uniform subpassInput input_normal;

vec3 calcFragPosRelCam(float depth, vec2 screen_pos, mat4 inv_proj);
float diffuse(vec3 normal, vec3 direction_to_light);
float specular(vec3 normal, vec3 direction_to_light, vec3 direction_to_cam);

void main() {

	// getting the position of the fragment from the depth buffer
	float depth = subpassLoad(input_depth).r;
	vec2 screen_pos = gl_FragCoord.xy * global.inv_viewport * 2.0 - 1.0;
	vec3 frag_pos_rel_cam = calcFragPosRelCam(depth, screen_pos, global.inv_proj);

	// getting the normal of the fragment from the normal buffer
	vec3 frag_normal_rel_cam = subpassLoad(input_normal).xyz;

	// calculating the lights intensity at the position of the fragment
	float d = length(frag_pos_rel_cam - light_pos_rel_cam);
	float intensity = 1 / (1 + light_lin_factor * d + light_qua_factor * d * d) - 0.01;
	intensity = max(intensity, 0);

	// calculating the tile map uv
	uvec2 material = uvec2(subpassLoad(input_material).xy * 255);
	vec2 cell_uv = subpassLoad(input_material).zw;
	vec2 tile_map_uv = (material + cell_uv * 0.99) * local.tile_map_unit;

	// getting the data from the tile map
	vec4 diffuse_roughness = texture(tile_map, vec3(tile_map_uv, 0));
	vec4 specular_metal = texture(tile_map, vec3(tile_map_uv, 1));

	// calculating the final color
	vec3 dir_to_light = normalize(light_pos_rel_cam - frag_pos_rel_cam);
	float phong = diffuse(frag_normal_rel_cam, dir_to_light) + specular(frag_normal_rel_cam, dir_to_light, normalize(-frag_pos_rel_cam)) + 0.1f;
	out_color = vec4(diffuse_roughness.rgb * light_color * phong * intensity, 0.0f);

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

float diffuse(vec3 normal, vec3 direction_to_light){
	
	return max(dot(direction_to_light, normal),0.0);
}

float specular(vec3 normal, vec3 direction_to_light, vec3 direction_to_cam){

	const float shininess = 128;
	const float specularStrength = 0.7;

	vec3 reflectDir = reflect(- direction_to_light, normal);
	float spec = pow(max(dot(direction_to_cam, reflectDir), 0.0), shininess);
	
	return specularStrength * spec;
}