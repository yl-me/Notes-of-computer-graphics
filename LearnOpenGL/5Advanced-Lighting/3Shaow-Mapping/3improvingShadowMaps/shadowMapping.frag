#version 330 core

out vec4 color;

in VS_OUT {
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
	vec4 FragPosLightSpace;
} fs_in;

uniform sampler2D diffuseTexture;
uniform sampler2D shadowMap;

uniform vec3 lightPos;
uniform vec3 cameraPos;

float shadowCalculation(vec4 fragPosLightSpace, vec3 lightDir)
{
	// perspective division
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	projCoords = projCoords * 0.5f + 0.5f;      // [-1,1] -> [0,1]
//	float closestDepth = texture(shadowMap, projCoords.xy).r;
	float currentDepth = projCoords.z;

	// shadow bias
	float bias = max(0.05f * dot(fs_in.Normal, lightDir), 0.005f);
	float shadow = 0.0f;
	vec2 texelSize = 1.0f / textureSize(shadowMap, 0);
	for (int x = -1; x <= 1; ++x) {
		for (int y = -1; y <= 1; ++y) {
			float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
			shadow += currentDepth - bias > pcfDepth ? 1.0f : 0.0f;	
		}
	}
	shadow /= 9.0f;
	if(projCoords.z > 1.0)
        shadow = 0.0;

	return shadow;
}

void main()
{
	// Blinn-Phong
	vec3 textureColor = texture(diffuseTexture, fs_in.TexCoords).rgb;
	vec3 normal = normalize(fs_in.Normal);
	vec3 lightColor = vec3(1.0f);
	// ambient
	vec3 ambient = textureColor * 0.15f;
	// diffuse
	vec3 lightDir = normalize(lightPos - fs_in.FragPos);
	float diff = max(dot(lightDir, normal), 0.0f);
	vec3 diffuse = diff * lightColor;
	// specular
	vec3 viewDir = normalize(cameraPos - fs_in.FragPos);
	vec3 halfway = normalize(lightDir + viewDir);
	float spec = pow(max(dot(halfway, normal), 0.0f), 64.0f);
	vec3 specular = spec * lightColor;
	
	// shadow calculation 
	float shadow = shadowCalculation(fs_in.FragPosLightSpace, lightDir);
	vec3 result = (ambient + (1.0f - shadow) * (diffuse + specular)) * textureColor;

	color = vec4(result, 1.0f);
}