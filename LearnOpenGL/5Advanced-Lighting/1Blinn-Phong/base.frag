#version 330 core

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

out vec4 color;

uniform sampler2D floorTexture;
uniform bool isBlinn;
uniform vec3 lightPos;
uniform vec3 cameraPos;

vec4 Phong(float shininess)
{
	vec3 textureColor = texture(floorTexture, fs_in.TexCoords).rgb;
	// ambient
    float ambientStrength = 0.05f;
	vec3 ambient = ambientStrength * textureColor;

	// diffuse
	vec3 norm = normalize(fs_in.Normal);
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
	float diff = max(dot(norm, lightDir), 0.0f);
	vec3 diffuse = diff * textureColor;

	// specular
	vec3 specularStrength = vec3(0.5f);
	vec3 viewDir = normalize(cameraPos - fs_in.FragPos);
	vec3 reflectDir = -lightDir - 2 * dot(norm, -lightDir) * norm;
	float spec = pow(max(dot(viewDir, reflectDir), 0.0f), shininess);
	vec3 specular = specularStrength * spec;

	vec3 result = ambient + diffuse + specular;

	color = vec4(result, 1.0f);

	return color;
}

vec4 BlinnPhong(float shininess)
{
	vec3 textureColor = texture(floorTexture, fs_in.TexCoords).rgb;
	// ambient
    float ambientStrength = 0.05f;
	vec3 ambient = ambientStrength * textureColor;

	// diffuse
	vec3 norm = normalize(fs_in.Normal);
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
	float diff = max(dot(norm, lightDir), 0.0f);
	vec3 diffuse = diff * textureColor;

	// specular
	vec3 specularStrength = vec3(0.5f);
	vec3 viewDir = normalize(cameraPos - fs_in.FragPos);

	// halfway
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(norm, halfwayDir), 0.0f), shininess);
	

	vec3 specular = specularStrength * spec;

	vec3 result = ambient + diffuse + specular;

	color = vec4(result, 1.0f);

	return color;
}

void main()
{
	float shininess = 8.0f;
	if (!isBlinn) {
		color = Phong(shininess);
	}
    else {
		color = BlinnPhong(shininess * 4);
	}
}