#version 330 core

out vec4 color;

in vec2 TexCoords;

uniform sampler2D hdrBuffer;
uniform float exposure;
uniform bool hdr;

void main()
{
	vec3 hdrColor = texture(hdrBuffer, TexCoords).rgb;
	float gamma = 2.2f;

	if(hdr)
    {
        // reinhard
        // vec3 result = hdrColor / (hdrColor + vec3(1.0f));
        // exposure
        vec3 result = vec3(1.0f) - exp(-hdrColor * exposure);
        // also gamma correct while we're at it       
        result = pow(result, vec3(1.0f / gamma));
        color = vec4(result, 1.0f);
    } else {
		// gamma correction
		vec3 mapped = pow(hdrColor, vec3(1.0f / gamma));

		color = vec4(mapped, 1.0f);
	}
}