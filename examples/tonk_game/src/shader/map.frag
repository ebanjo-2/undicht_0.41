#version 450

layout(location = 0) out vec4 out_color;

layout(location = 0) in vec2 uv;

layout(binding = 0) uniform sampler2D tile_map;


void main() {
    
    out_color = texture(tile_map, uv);
    
}