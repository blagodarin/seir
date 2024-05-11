#version 450

layout(push_constant) uniform PushConstants {
	mat4 matrix;
} push;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec4 inColor;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outTexCoord;

void main()
{
	gl_Position = push.matrix * vec4(inPosition, 0, 1);
	outColor = inColor;
	outTexCoord = inTexCoord;
}
