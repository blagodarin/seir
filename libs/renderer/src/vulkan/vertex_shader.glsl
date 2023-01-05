#version 450

layout(binding = 0) uniform UniformBufferObject {
	mat4 matrix;
} ubo;

layout(push_constant) uniform PushConstants {
	mat4 matrix;
} push;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 outColor;
layout(location = 1) out vec2 outTexCoord;

void main()
{
	gl_Position = push.matrix * vec4(inPosition, 1.0);
	outColor = inColor;
	outTexCoord = inTexCoord;
}
