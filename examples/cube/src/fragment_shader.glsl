#version 450

layout(set = 0, binding = 0) uniform sampler2D texSampler;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec4 outColor;

void main()
{
	vec3 n = normalize(inNormal);
	vec3 l = normalize(vec3(0, -5, 1) - inPosition);
	vec3 c = texture(texSampler, inTexCoord).rgb * inColor;
	vec3 ambient = c * 0.125;
	vec3 diffuse = c * 0.875 * max(dot(n, l), 0);
	outColor = vec4(ambient + diffuse, 1);
}
