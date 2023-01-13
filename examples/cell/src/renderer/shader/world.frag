#version 450

layout(location = 0) out uvec4 out_color;

layout(location = 0) in flat uint face_id;
layout(location = 1) in flat uvec2 material;

layout(binding = 0) uniform GlobalUBO {
	mat4 view;
	mat4 proj;
	mat4 inv_view;
	mat4 inv_proj;
} global;


layout(binding = 2) uniform sampler2D tile_map;

void main() {

	out_color = uvec4(material, face_id, 0);

	/*// out_color = texture(tile_map, uv);
	if((face_id & 0x01) != 0)
		out_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
	else if((face_id & 0x02) != 0)
		out_color = vec4(1.0f, 1.0f, 0.0f, 1.0f);
	else if((face_id & 0x04) != 0)
		out_color = vec4(0.0f, 1.0f, 0.0f, 1.0f);
	else if((face_id & 0x08) != 0)
		out_color = vec4(0.0f, 0.0f, 1.0f, 1.0f);
	else if((face_id & 0x10) != 0)
		out_color = vec4(1.0f, 0.0f, 1.0f, 1.0f);
	else // if((face_id & 0x20) != 0)
		out_color = vec4(1.0f, 0.0f, 1.0f, 1.0f);*/
}
