#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
out vec4 color;

uniform vec3 cameraPos;

struct Material {
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};
uniform Material material;

struct DirLight {
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
uniform DirLight dirLight;

vec3 CalcDirLight(DirLight dirLight, vec3 norm, vec3 viewDir);

struct PointLight {
	vec3 direction;
	vec3 position;
	float constant;
	float linear;
	float quadratic;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform PointLight pointLight;

vec3 CalcPointLight(PointLight pointLight, vec3 norm, vec3 FragPos, vec3 viewDir);

struct SpotLight {
	vec3 position;
	vec3 direction;
	float cutOff;
	float outerCutOff;
	float constant;
	float linear;
	float quadratic;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform SpotLight spotLight;

vec3 CalcSpotLight(SpotLight spotLight, vec3 norm, vec3 FragPos, vec3 viewDir);

void main()
{
	vec3 normal = normalize(Normal);
	vec3 viewDir = normalize(cameraPos - FragPos);
	vec3 result;

	result = CalcDirLight(dirLight, normal, viewDir);


	result += CalcPointLight(pointLight, normal, FragPos, viewDir);

	result += CalcSpotLight(spotLight, normal, FragPos, viewDir);

	color = vec4(result, 1.0f);
}

vec3 CalcDirLight(DirLight dirLight, vec3 norm, vec3 viewDir)
{
	// ambient
	vec3 ambient = dirLight.ambient * vec3(texture(material.diffuse, TexCoords));

	// diffuse
	vec3 lightDir = normalize(-dirLight.direction);
	float diff = max(dot(norm, lightDir), 0.0f);
	vec3 diffuse = diff * dirLight.diffuse * vec3(texture(material.diffuse, TexCoords));

	// specular
	vec3 reflectDir = -lightDir - 2 * dot(norm, -lightDir) * norm;
	float spec = pow(max(dot(viewDir, reflectDir), 0.0f), material.shininess);
	vec3 specular = spec * dirLight.specular * vec3(texture(material.specular, TexCoords));

	return ambient + diffuse + specular;
}

vec3 CalcPointLight(PointLight pointLight, vec3 norm, vec3 FragPos, vec3 viewDir)
{
	// ambient
	vec3 ambient = pointLight.ambient * vec3(texture(material.diffuse, TexCoords));

	// diffuse
	vec3 lightDir = normalize(pointLight.position - FragPos);
	float diff = max(dot(norm, lightDir), 0.0f);
	vec3 diffuse = diff * pointLight.diffuse * vec3(texture(material.diffuse, TexCoords));

	// specular
	vec3 reflectDir = -lightDir - 2 * dot(norm, -lightDir) * norm;
	float spec = pow(max(dot(viewDir, reflectDir), 0.0f), material.shininess);
	vec3 specular = spec * pointLight.specular * vec3(texture(material.specular, TexCoords));

	// attenuation
	float distance = length(pointLight.position - FragPos);
	float attenuation = 1.0f / (pointLight.constant + pointLight.linear * distance + pointLight.quadratic * (distance * distance));

	return ambient + (diffuse + specular) * attenuation;
}

vec3 CalcSpotLight(SpotLight spotLight, vec3 norm, vec3 FragPos, vec3 viewDir)
{
	// ambient
	vec3 ambient = spotLight.ambient * vec3(texture(material.diffuse, TexCoords));

	// diffuse
	vec3 lightDir = normalize(spotLight.position - FragPos);
	float diff = max(dot(norm, lightDir), 0.0f);
	vec3 diffuse = diff * spotLight.diffuse * vec3(texture(material.diffuse, TexCoords));

	// specular
	vec3 reflectDir = -lightDir - 2 * dot(norm, -lightDir) * norm;
	float spec = pow(max(dot(viewDir, reflectDir), 0.0f), material.shininess);
	vec3 specular = spec * spotLight.specular * vec3(texture(material.specular, TexCoords));

	// attenuation
	float distance = length(spotLight.position - FragPos);
	float attenuation = 1.0f / (spotLight.constant + spotLight.linear * distance + spotLight.quadratic * (distance * distance));

	// indensity
	float theta = dot(lightDir, normalize(-spotLight.direction));
	float espilon = spotLight.cutOff - spotLight.outerCutOff;
	float indensity = clamp((theta - spotLight.outerCutOff) / espilon, 0.0f, 1.0f);

	return ambient + (diffuse + specular) * attenuation * indensity;
}