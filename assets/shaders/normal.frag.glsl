#version 330 core

in vec3 vNor;
in vec2 vUv;

uniform vec3 uLightDir;
uniform vec3 uColor;
uniform sampler2D uAlbedo;
uniform int uHasAlbedo;

out vec4 FragColor;

void main()
{
    vec3 N = normalize(vNor);
    float ndl = max(dot(N, normalize(uLightDir)), 0.0);
    float ambient = 0.2;

    vec3 base = uColor;
    if (uHasAlbedo != 0)
    {
        base *= texture(uAlbedo, vUv).rgb;
    }

    vec3 lit = base * (ambient + (1.0 - ambient) * ndl);
    FragColor = vec4(lit, 1.0);
}
