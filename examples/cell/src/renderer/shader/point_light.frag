#version 450

layout(location = 0) out vec4 out_color;

layout(location = 0) in vec3 light_color;
layout(location = 1) in vec3 light_pos_rel_cam;

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

// reading the inputs
vec3 calcFragPosRelCam(float depth);
vec2 getTileMapUV();

// pbr math functions
vec3 fresnelSchlick(float cosTheta, vec3 F0);
float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);

const float PI = 3.14159265359;

void main() {

	// getting the position and normal of the fragment (in view space)
    vec4 normal_depth = subpassLoad(input_normal);
	vec3 frag_pos_rel_cam = calcFragPosRelCam(normal_depth.a);
	vec3 frag_normal_rel_cam = normal_depth.xyz;
	vec3 N = normalize(frag_normal_rel_cam);

	// reading the material properties
	vec2 tile_map_uv = getTileMapUV();
	vec4 albedo_roughness = texture(tile_map, vec3(tile_map_uv, 0));
	vec4 normal_metal = texture(tile_map, vec3(tile_map_uv, 1));
	vec3 albedo = albedo_roughness.rgb;
	float roughness = albedo_roughness.a;
	float metallic = normal_metal.a;
	//float roughness = 0.1f;
	//float metallic = 0.0f;

	// frequently used vectors
	vec3 V = normalize(-frag_pos_rel_cam); // cam sits at (0,0,0) in the view space
	vec3 L = normalize(light_pos_rel_cam - frag_pos_rel_cam);
	vec3 H = normalize(V + L); // half vector between the direction to the light and to the cam

	float distance    = length(light_pos_rel_cam - frag_pos_rel_cam);
	float attenuation = 1.0 / (distance * distance);
    vec3 radiance     = max(light_color * attenuation , 0.0);
    
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // cook-torrance brdf
    float NDF = DistributionGGX(N, H, roughness);        
    float G   = GeometrySmith(N, V, L, roughness);      
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);       
        
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;
        
    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular     = numerator / denominator;
            
    // add to outgoing radiance Lo
    float NdotL = max(dot(N, L), 0.0);
    out_color = vec4((kD * albedo / PI + specular) * radiance * NdotL, 0.0f); 

}

//////////////////////////////////////////// reading the inputs ////////////////////////////////////////////

vec3 calcFragPosRelCam(float depth) {
	// reconstructing the fragments position in view space (relative to the camera)
	// from the depth value read from the depth buffer

	// reading the depth value from the depth input texture
	// float depth = subpassLoad(input_depth).r;
	vec2 screen_pos = gl_FragCoord.xy * global.inv_viewport * 2.0 - 1.0;

    // thanks for the math 
    // https://stackoverflow.com/questions/32227283/getting-world-position-from-depth-buffer-value
    vec4 clipSpacePosition = vec4(screen_pos, depth, 1.0);
	clipSpacePosition.y = -clipSpacePosition.y;
    vec4 viewSpacePosition = global.inv_proj * clipSpacePosition;

    // Perspective division
    viewSpacePosition /= viewSpacePosition.w;

	return viewSpacePosition.xyz;
}

vec2 getTileMapUV() {

	uvec2 material = uvec2(subpassLoad(input_material).xy * 255);
	vec2 cell_uv = subpassLoad(input_material).zw;
	vec2 tile_map_uv = (material + cell_uv * 0.99) * local.tile_map_unit;

	return tile_map_uv;
}


//////////////////////////////////////////// pbr math functions ////////////////////////////////////////////

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
	// calculate the ratio between specular and diffuse reflection
	// materials with a higher "metalness" absorb fewer light, so the specular reflection is greater
	// at greater reflection angles the specular part also gets bigger
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {

    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {

    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {

    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}