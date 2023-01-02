#version 450

layout(location = 0) out vec4 out_color;

layout(location = 0) in vec2 uv;

layout (binding = 1) uniform sampler2D tile_map;
layout (input_attachment_index = 0, set = 0, binding = 2) uniform subpassInput input_depth;
layout (input_attachment_index = 1, set = 0, binding = 3) uniform subpassInput input_color;

void main() {
	
 	//out_color = subpassLoad(input_depth).rrrr;
	out_color = subpassLoad(input_color);
	//out_color = texture(tile_map, uv);
}
