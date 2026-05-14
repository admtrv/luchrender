#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 uModel;
uniform mat4 uLightVP;

void main()
{
    gl_Position = uLightVP * uModel * vec4(aPos, 1.0);
}
