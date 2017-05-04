#version 330 core
in vec3 Normal;
in vec3 Position;

out vec4 color;

uniform samplerCube texture1;
uniform vec3 cameraPos;

void main()
{
	vec3 I = normalize(Position - cameraPos);
	vec3 R = I - 2 * dot(normalize(Normal), I) * normalize(Normal);
	color = vec4(texture(texture1, R).rgb, 1.0f);
}
