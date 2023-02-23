#version 450

layout(location = 0) out vec4 out_color;

layout(location = 0) in vec3 light_dir_rel_cam;

// global inputs
layout(set = 0, binding = 0) uniform GlobalUBO {
	mat4 view;
	mat4 proj;
	mat4 inv_view;
	mat4 inv_proj;
	vec2 viewport;
	vec2 inv_viewport;
} global;

// inputs for all light renderers
layout(set = 1, binding = 0) uniform LocalUBO {
	vec2 tile_map_unit;
    ivec2 shadow_offset_texture_size;
    ivec2 shadow_offset_filter_size;
} local;

//layout (set = 1, binding = 1) uniform sampler2DArray tile_map;
layout (set = 1, binding = 1) uniform sampler3D shadow_offsets;
layout (set = 1, input_attachment_index = 0, binding = 2) uniform subpassInput input_albedo_roughness;
layout (set = 1, input_attachment_index = 1, binding = 3) uniform subpassInput input_normal_metalness; // contains the normal data + metalness
layout (set = 1, input_attachment_index = 1, binding = 4) uniform subpassInput input_position_rel_cam; 
layout (set = 1, input_attachment_index = 2, binding = 5) uniform subpassInput input_shadow_map_pos; // position of the fragment on the shadow map

// inputs specific to this type of light renderer
layout(set = 2, binding = 0) uniform LightUBO {
	vec3 color;
    vec3 direction;
    vec2 shadow_map_unit;
} light;

layout (set = 2, binding = 1) uniform sampler2D shadow_map;


// shadow mapping
float isInLight(vec3 N, vec3 L);

// pbr math functions
vec3 fresnelSchlick(float cosTheta, vec3 F0);
float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);

const float PI = 3.14159265359;


void main() {

	// loading data from previous subpasses
    vec4 albedo_roughness = subpassLoad(input_albedo_roughness);
    vec4 normal_metalness = subpassLoad(input_normal_metalness);
	vec3 frag_pos_rel_cam = subpassLoad(input_position_rel_cam).xyz; // calcFragPosRelCam(normal_depth.a);

	// material properties
	vec3 albedo = albedo_roughness.rgb;
	float roughness = albedo_roughness.a;
	float metallic = normal_metalness.a;

	// frequently used vectors
    vec3 N = normalize(normal_metalness.xyz);
	vec3 V = normalize(-frag_pos_rel_cam); // cam sits at (0,0,0) in the view space
	vec3 L = normalize(-light_dir_rel_cam);
	vec3 H = normalize(V + L); // half vector between the direction to the light and to the cam

	float attenuation = 1.0; // directional light, no attenuation
    vec3 radiance     = max(light.color * attenuation , 0.0);
    
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
    out_color = vec4((kD * albedo / PI + specular) * radiance * NdotL, 0.0f) * isInLight(N, L);

}

////////////////////////////////////////////// shadow mapping ////////////////////////////////////////////

float isInLight(vec3 N, vec3 L) {

    vec4 pos_on_shadow_map = subpassLoad(input_shadow_map_pos); // pos of this fragment on the shadow 
    vec3 shadow_coords = vec3(0.0, 0.0, min(pos_on_shadow_map.z, 1.0f)); // going to contain the position on the shadow map per sample
    ivec2 current_filter = ivec2(mod(gl_FragCoord.xy, vec2(local.shadow_offset_texture_size))); // filter for the current fragment
    ivec3 offset_coord = ivec3(0, current_filter); // going to be used to access a offset from the 3D offset texture

    float random_sample_radius = 1.0f;
    float bias = 0.0; // mix(0.00005, 0.0, dot(N, L));
    int shadow_sum = 0;

    // ring 0 contains the offsets that are the furthest from the fragment
    int ring;
    int total_rings = local.shadow_offset_filter_size.x;
    int samples_per_ring = local.shadow_offset_filter_size.y;

    for(ring = 0; ring < total_rings; ring++) {

        int ring_sum = 0;

        // testing the samples on the current ring (2 at a time)
        for(int i = 0; i < samples_per_ring / 2; i++) {

            offset_coord.x = (ring * samples_per_ring / 2 + i); // 2 samples per texel (rg and ba)
            vec4 offsets = texelFetch(shadow_offsets, offset_coord, 0) * random_sample_radius;
            float dist;

            // first sample
            shadow_coords.xy = pos_on_shadow_map.xy + offsets.rg * light.shadow_map_unit;
            dist = shadow_coords.z - texture(shadow_map, shadow_coords.xy).r;
            ring_sum += ((dist > bias) ? 1 : 0);

            // second sample
            shadow_coords.xy = pos_on_shadow_map.xy + offsets.ba * light.shadow_map_unit;
            dist = shadow_coords.z - texture(shadow_map, shadow_coords.xy).r;
            ring_sum += ((dist > bias) ? 1 : 0);

        }

        // adding the result of testing from this ring to the total shadow sum
        shadow_sum += ring_sum;

        // if all of the samples on this ring were in shadow or light -> not going to test the inner rings
        if((ring_sum == samples_per_ring) || (ring_sum == 0)) {
            ring++;
            break;
        }

    }

    float shadow = float(shadow_sum) / float(ring  * samples_per_ring);

    return 1.0f - shadow; // light;
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
    float NdotH  = max(dot(N, H), 0.0f);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {

    float r = (roughness + 1.0f);
    float k = (r*r) / 8.0f;

    float num   = NdotV;
    float denom = NdotV * (1.0f - k) + k;
	
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {

    float NdotV = max(dot(N, V), 0.0f);
    float NdotL = max(dot(N, L), 0.0f);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}