#version 450

layout(location = 0) out vec4 out_color;

layout(location = 0) in vec2 uv;


layout(set = 1, binding = 0) uniform LocalUBO {
	float exposure;
} local;

layout (set = 1, input_attachment_index = 0, binding = 1) uniform subpassInput input_light;


void main() {

	// using the gamma correction / exposure tone mapping from https://learnopengl.com/Advanced-Lighting/HDR
    vec3 hdr_color = subpassLoad(input_light).rgb;
    vec3 mapped = vec3(1.0) - exp(-hdr_color * local.exposure); // exposure tone mapping

    // no gamma correction, because the swapchain image format should be an sRGB image
    // so gamma correction will be done automatically
    //mapped = pow(mapped, vec3(1.0 / local.gamma)); // gamma correction 
  

    //out_color = vec4(hdr_color, 1.0);
    out_color = vec4(mapped, 1.0);
}