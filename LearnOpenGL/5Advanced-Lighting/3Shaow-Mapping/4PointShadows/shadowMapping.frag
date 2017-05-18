#version 330 core

out vec4 color;

in VS_OUT {
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
} fs_in;

uniform sampler2D diffuseTexture;
uniform samplerCube depthMap;
uniform float far_plane;
uniform bool doShadow;

uniform vec3 lightPos;
uniform vec3 cameraPos;

float shadowCalculation(vec3 fragPos)
{
	vec3 fragToLight = fragPos - lightPos;
	float closestDepth = texture(depthMap, fragToLight).r;
	closestDepth *= far_plane;
	float currentDepth = length(fragToLight);

	// shadow bias
	float bias = max(0.05f * dot(fs_in.Normal, -fragToLight), 0.005f);
	float shadow = currentDepth - bias > closestDepth ? 1.0f : 0.0f;

	return shadow;
}

void main()
{
	// Blinn-Phong
	vec3 textureColor = texture(diffuseTexture, fs_in.TexCoords).rgb;
	vec3 normal = normalize(fs_in.Normal);
	vec3 lightColor = vec3(0.3f);
	// ambient
	vec3 ambient = textureColor * 0.3f;
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
	float shadow = doShadow ? shadowCalculation(fs_in.FragPos) : 0.0f;
	vec3 result = (ambient + (1.0f - shadow) * (diffuse + specular)) * textureColor;

	color = vec4(result, 1.0f);
}