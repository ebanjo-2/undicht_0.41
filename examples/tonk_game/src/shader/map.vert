#version 450

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aWorldPos;
layout(location = 2) in vec2 aUv;

layout(location = 0) out vec2 uv;

layout(binding = 1) uniform UniformBufferObject {
	float aspect_ratio;
	float zoom_factor;
    vec2 map_center;
} ubo;


void main() {

    vec2 tile_size = vec2(1.0f / 64.0f);

    uv = aUv + aPos * tile_size;

    vec2 world_pos = (aPos + aWorldPos);
    vec2 screen_pos = (world_pos - ubo.map_center) * vec2(1.0f, ubo.aspect_ratio) * ubo.zoom_factor;
    gl_Position = vec4(screen_pos, 0.0f, 1.0f);
}