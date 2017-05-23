#version 330 core

out vec4 color;

in VS_OUT {
	vec3 FragPos;
	vec2 TexCoords;
	vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} fs_in;

uniform sampler2D diffuseTexture;
uniform sampler2D normalTexture;

void main()
{
	// Blinn-Phong
	vec3 textureColor = texture(diffuseTexture, fs_in.TexCoords).rgb;
	vec3 normal = vec3(0.0f);
	normal = texture(normalTexture, fs_in.TexCoords).rgb;
	normal = normalize(normal * 2.0f - 1.0f);   // [0,1]->[-1,1]
	// ambient
	vec3 ambient = textureColor * 0.1f;
	// diffuse
	vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
	float diff = max(dot(lightDir, normal), 0.0f);
	vec3 diffuse = diff * textureColor;
	// specular
	vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
	vec3 halfway = normalize(lightDir + viewDir);
	float spec = pow(max(dot(halfway, normal), 0.0f), 32.0f);
	vec3 specular = spec * vec3(0.2f);

	vec3 result = ambient + diffuse + specular;

	color = vec4(result, 1.0f);
}