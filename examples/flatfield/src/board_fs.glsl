#version 450

layout(set = 0, binding = 0) uniform sampler2D texSampler;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec4 outColor;

void main()
{
	vec3 n = normalize(inNormal);
	vec3 l = normalize(vec3(0, 0, 8) - inPosition);
	vec3 r = -reflect(l, n);
	vec3 diffuse = vec3(1, 1, 1) * max(dot(n, l), 0);
	vec4 c = texture(texSampler, inTexCoord);
	outColor = vec4(diffuse, 1) * c;
}
