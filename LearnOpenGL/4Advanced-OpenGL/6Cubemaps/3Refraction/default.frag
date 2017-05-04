#version 330 core
in vec3 Normal;
in vec3 Position;

out vec4 color;

uniform samplerCube texture1;
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
//	reflect
//	vec3 I = normalize(Position - cameraPos);
//	vec3 Reflect = reflect(I, normalize(Normal));
//	vec3 Reflect = myReflect(I, normalize(Normal));
//	color = vec4(texture(texture1, Reflect).rgb, 1.0f);

//	float ration = 1.00f / 1.00f;      // air
//	float ration = 1.00f / 1.33f;       // water
//	float ration = 1.00f / 1.309f;       // ice
//	float ration = 1.00f / 1.52f;       // glass
	float ration = 1.00f / 2.42f;       // diamond
	vec3 I = normalize(Position - cameraPos);
//	vec3 Refract = refract(I, normalize(Normal), ration);
	vec3 Refract = myRefract(I, normalize(Normal), ration);
	color = vec4(texture(texture1, Refract).rgb, 1.0f);
}