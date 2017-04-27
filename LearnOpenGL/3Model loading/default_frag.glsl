#version 330 core

struct Material {
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};
uniform Material material;

struct Light {
	vec3 direction;
	vec3 position;
	float cutOff;
	float outerCutOff;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;        // Attenuation
	float linear;
	float quadratic;
};
uniform Light light;


in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

out vec4 color;

uniform vec3 cameraPos;

void main()
{
	// ambient
	vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));

	// diffuse
	vec3 norm = normalize(Normal);
//  vec3 lightDir = normalize(-light.direction);
	vec3 lightDir = normalize(light.position - FragPos);
	float diff = max(dot(norm, lightDir), 0.0f);
	vec3 diffuse = diff * light.diffuse * vec3(texture(material.diffuse, TexCoords));

	// specular
	vec3 viewDir = normalize(cameraPos - FragPos);
	vec3 reflectDir = -lightDir - 2 * dot(norm, -lightDir) * norm;
//	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0f), material.shininess);
	vec3 specular = spec * light.specular * vec3(texture(material.specular, TexCoords));

	// attenuation
	float distance = length(light.position - FragPos);
	float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

	float theta = dot(lightDir, normalize(-light.direction));
	float espilon = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / espilon, 0.0f, 1.0f);  // clamp把值固定到指定的范围

	vec3 result = ambient + (diffuse + specular) * attenuation * intensity;

	color = vec4(result, 1.0f);
}
