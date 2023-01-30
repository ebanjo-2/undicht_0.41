#version 450

layout(location = 0) out vec4 out_material;
layout(location = 1) out vec4 out_normal;
layout(location = 2) out vec4 out_light;

layout(location = 0) in flat uint face_id;
layout(location = 1) in flat vec2 material;
layout(location = 2) in vec3 pos_rel_cam;
layout(location = 3) in vec3 normal_rel_cam;
layout(location = 4) in vec2 cell_uv;
layout(location = 5) in vec4 pos_on_shadow_map;

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
} local;

layout(set = 1, binding = 1) uniform sampler2DArray tile_map;
layout(set = 1, binding = 2) uniform sampler2D shadow_map;

layout(set = 2, binding = 0) uniform ChunkUBO {
	ivec3 pos;
} chunk;

float isInLight(vec4 pos_on_shadow_map);

void main() {

	out_material.xy = material;
	out_material.zw = fract(cell_uv);
	out_normal = vec4(normal_rel_cam, gl_FragCoord.z);
	out_light = vec4(0.0f, 0.0f, 0.0f, isInLight(pos_on_shadow_map)); // store isInLight in the alpha channel;

	//out_light = vec4(pow(texture(shadow_map, global.inv_viewport * gl_FragCoord.xy).r, 4));
}

float isInLight(vec4 pos_on_shadow_map) {

    // perform perspective divide
    vec3 projCoords = pos_on_shadow_map.xyz;// / pos_on_shadow_map.w;
    // transform to [0,1] range
    projCoords.xy = projCoords.xy * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadow_map, projCoords.xy).r;
    // get depth of current fragment from light's perspective
    float currentDepth = min(1.0f, projCoords.z);
    // check whether current frag pos is in shadow
    float bias = 0.005;
    float light = (currentDepth - bias <= closestDepth)  ? 1.0 : 0.0;  

    return light;
}