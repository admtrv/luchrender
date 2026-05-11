#version 330 core

layout (location=0) in vec3 aPos;
layout (location=1) in vec3 aNor;
layout (location=2) in vec2 aUv;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;

out vec3 vNor;
out vec2 vUv;

void main()
{
    mat3 nmat = mat3(uModel);
    vNor = normalize(nmat * aNor);
    vUv = aUv;
    gl_Position = uProj * uView * uModel * vec4(aPos, 1.0);
}
