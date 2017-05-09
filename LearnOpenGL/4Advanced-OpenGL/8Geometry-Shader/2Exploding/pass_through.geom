#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec2 texCoords;
} gs_in[];

out vec2 TexCoords;

uniform float time;

vec3 GetNormal()
{
	vec3 p1 = vec3(gl_in[0].gl_Position - gl_in[1].gl_Position);
	vec3 p2 = vec3(gl_in[2].gl_Position - gl_in[1].gl_Position);
	return normalize(cross(p1, p2));
}

vec4 explode(vec4 position, vec3 normal)
{
	float magnitude = 2.0f;
	vec3 direction = normal * ((sin(time) + 1.0f) / 5.0f) * magnitude;
	return position + vec4(direction, 0.0f);
}

void main()
{
	vec3 normal = GetNormal();
	gl_Position = explode(gl_in[0].gl_Position, normal);
    TexCoords = gs_in[0].texCoords;
    EmitVertex();
    gl_Position = explode(gl_in[1].gl_Position, normal);
    TexCoords = gs_in[1].texCoords;
    EmitVertex();
    gl_Position = explode(gl_in[2].gl_Position, normal);
    TexCoords = gs_in[2].texCoords;
    EmitVertex();
    EndPrimitive();
}