#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 uView;
uniform mat4 uProj;

out vec3 vDir;

void main()
{
    vDir = aPos;
    mat4 viewNoTrans = mat4(mat3(uView));
    vec4 pos = uProj * viewNoTrans * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}
