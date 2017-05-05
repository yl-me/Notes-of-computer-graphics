#version 330 core
in vec3 Normal;
in vec3 Position;
in vec2 TexCoords;

out vec4 color;

uniform sampler2D texture_diffuse;
uniform sampler2D texture_reflection;
uniform samplerCube skybox;
uniform vec3 cameraPos;


vec3 myRefract(vec3 I, vec3 Normal, float ration)
{
	float k = 1.0 - ration * ration * (1.0 - dot(Normal, I) * dot(Normal, I));
	vec3 R;
	if (k < 0.0)
		R = vec3(0.0);       // or genDType(0.0)
	else
		R = -Normal * sqrt(k) + ration * (I - dot(Normal, I) * Normal);
	return R;
}

vec3 myReflect(vec3 I, vec3 Normal)
{
	vec3 R = I - 2 * dot(Normal, I) * Normal;
	return R;
}

void main()
{
	vec4 diffuse_color = texture(texture_diffuse, TexCoords);
	vec3 I = normalize(Position - cameraPos);
	vec3 R = myReflect(I, normalize(Normal));
	float reflect_intensity = texture(texture_reflection, TexCoords).r;
	vec4 reflect_color;
	if (reflect_intensity > 0.1f) {
		reflect_color = texture(skybox, R) * reflect_intensity;
	}
	color = diffuse_color + reflect_color;
}