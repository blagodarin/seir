#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec4 outColor;

void main()
{
	vec3 n = normalize(inNormal);
	vec3 l = normalize(vec3(16, 32, 64) - inPosition);
	vec3 r = -reflect(l, n);
	vec3 ambient = vec3(0.1, 0.0, 0.0);
	vec3 diffuse = vec3(0.5, 0.1, 0.1) * max(dot(n, l), 0);
	outColor = vec4(ambient + diffuse, 1);
}
