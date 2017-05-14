#version 330 core

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

out vec4 color;

uniform sampler2D floorTexture;
uniform bool isBlinn;
uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];;
uniform vec3 cameraPos;
uniform bool gamma;

vec3 Phong_BlinnPhong(vec3 lightPos, vec3 lightColor)
{
	// ambient
    float ambientStrength = 0.05f;
	vec3 ambient = ambientStrength * lightColor;

	// diffuse
	vec3 norm = normalize(fs_in.Normal);
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
	float diff = max(dot(norm, lightDir), 0.0f);
	vec3 diffuse = diff * lightColor;


	// specular
	vec3 specularStrength = vec3(0.5f);
	vec3 viewDir = normalize(cameraPos - fs_in.FragPos);
	
	float shininess = 8.0f;
	float spec = 0.0f;
	if (isBlinn) {
		// halfway
		vec3 halfwayDir = normalize(lightDir + viewDir);
		spec = pow(max(dot(norm, halfwayDir), 0.0f), shininess * 4);
	}
	else {
		vec3 reflectDir = -lightDir - 2 * dot(norm, -lightDir) * norm;
		spec = pow(max(dot(viewDir, reflectDir), 0.0f), shininess);
	}
	vec3 specular = specularStrength * spec;

	// attenuation
	float max_distance = 1.5;
    float distance = length(lightPos - fs_in.FragPos);
    float attenuation = 1.0 / (gamma ? distance * distance : distance);

	vec3 result = ambient + (diffuse + specular) * attenuation;

	return result;
}

void main()
{
	vec3 textureColor = texture(floorTexture, fs_in.TexCoords).rgb;
	vec3 lighting = vec3(0.0f);
	for(int i = 0; i < 4; ++i)
        lighting += Phong_BlinnPhong(lightPositions[i], lightColors[i]);
	textureColor *= lighting;

	// gamma
	if (gamma) {
		float gammaValue = 2.2f;
		textureColor.rgb = pow(textureColor.rgb, vec3(1 / gammaValue));
	}
	color = vec4(textureColor, 1.0f);
}