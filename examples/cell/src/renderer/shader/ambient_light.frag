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

// layout (set = 1, binding = 1) uniform sampler3D shadow_offsets; // unused in this shader
layout (set = 1, input_attachment_index = 0, binding = 2) uniform subpassInput input_albedo_roughness;
layout (set = 1, input_attachment_index = 1, binding = 3) uniform subpassInput input_normal_metalness; // contains the normal data + metalness
layout (set = 1, input_attachment_index = 1, binding = 4) uniform subpassInput input_position_rel_cam; 
// layout (set = 1, input_attachment_index = 2, binding = 5) uniform subpassInput input_shadow_map_pos; // position of the fragment on the shadow map

layout (set = 2, binding = 0) uniform samplerCube env_cube_map;
layout (set = 2, binding = 1) uniform samplerCube irradiance_map;
layout (set = 2, binding = 2) uniform samplerCube specular_prefilter_map;
layout (set = 2, binding = 3) uniform sampler2D brdf_lut; // lut = look up texture


// pbr math functions
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);
float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);

const float PI = 3.14159265359;

void main() {

	// loading data from previous subpasses
    vec4 albedo_roughness = subpassLoad(input_albedo_roughness);
    vec4 normal_metalness = subpassLoad(input_normal_metalness);
	vec3 frag_pos_rel_cam = subpassLoad(input_position_rel_cam).xyz;

	if(length(frag_pos_rel_cam) == 0.0) { // display the sky box
		out_color = texture(env_cube_map, sample_dir);
		//out_color = textureLod(env_cube_map, sample_dir, 1.0);
		return;
    }

	// material properties
	vec3 albedo = albedo_roughness.rgb;
	float roughness = albedo_roughness.a;
	float metallic = normal_metalness.a;

	// frequently used vectors
	mat3 inv_view_rotation = mat3(global.inv_view); // sampling the environment maps requires world space directions, not view space
    vec3 N = inv_view_rotation * normalize(normal_metalness.xyz);
	vec3 V = inv_view_rotation * normalize(-frag_pos_rel_cam); // cam sits at (0,0,0) in the view space

	vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

	vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= (1.0 - metallic);
	
	// sample irradiance map
	vec3 irradiance = texture(irradiance_map, N).rgb;
	vec3 diffuse    = irradiance * albedo;

	// sample specular prefilter map
	vec3 R = reflect(-V, N);
    const float MAX_REFLECTION_LOD = 4.0; // number of mip levels in the prefiltered specular map minus 1
    vec3 prefilteredColor = textureLod(specular_prefilter_map, R,  roughness * MAX_REFLECTION_LOD).rgb;

	// sample BRDF lookup texture
	vec2 envBRDF  = texture(brdf_lut, vec2(max(dot(N, V), 0.0), roughness)).rg;
	vec3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);

	vec3 ambient    = kD * diffuse + specular;
	out_color = vec4(ambient, 0.0);
}

//////////////////////////////////////////// pbr math functions ////////////////////////////////////////////

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {

    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
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