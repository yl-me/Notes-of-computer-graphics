#version 330 core
layout (location = 0) in vec3 position;

uniform mat4 view;
uniform mat4 projection;

out vec3 TexCoords;

void main()
{
    vec4 pos = projection * view * vec4(position, 1.0f);
    gl_Position = pos.xyww;
    TexCoords = position;
}