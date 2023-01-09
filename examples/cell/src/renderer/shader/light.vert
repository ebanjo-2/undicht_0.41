#version 450

// per vertex data
layout(location = 0) in vec3 aPos;

// per light data
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec3 aLightPos;
layout(location = 3) in float aIntensity;


layout(binding = 0) uniform GlobalUBO {
	mat4 proj;
	mat4 view;
} global;



void main() {

	gl_Position = global.proj * global.view * vec4(aPos + aLightPos, 1.0f);
	
	// positive y is up, change my mind
	gl_Position.y = -gl_Position.y;
}
