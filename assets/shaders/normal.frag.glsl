#version 330 core

#define MAX_POINT_LIGHTS 3
#define MAX_SPOT_LIGHTS 3

in vec3 vWorldPos;
in vec3 vNor;
in vec2 vUv;

uniform vec3 uColor;
uniform sampler2D uAlbedo;
uniform int uHasAlbedo;

uniform vec3 uMatSpecular;
uniform float uMatShininess;
uniform vec3 uMatEmissive;
uniform sampler2D uSpecularMap;
uniform int uHasSpecMap;

uniform vec3 uCameraPos;

uniform vec3 uAmbientColor;

uniform int uHasDirLight;
uniform vec3 uDirLightDir;
uniform vec3 uDirLightColor;
uniform int uDirCastsShadow;
uniform mat4 uDirLightVP;
uniform sampler2D uDirShadowMap;

struct PointLight {
    vec3 position;
    vec3 color;
    float range;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 color;
    float range;
    float innerCos;
    float outerCos;
};

uniform int uPointCount;
uniform PointLight uPointLights[MAX_POINT_LIGHTS];

uniform int uSpotCount;
uniform SpotLight uSpotLights[MAX_SPOT_LIGHTS];
uniform mat4 uSpotLightVP[MAX_SPOT_LIGHTS];
uniform int uSpotCastsShadow[MAX_SPOT_LIGHTS];
uniform sampler2D uSpotShadowMap[MAX_SPOT_LIGHTS];

out vec4 FragColor;

// smooth distance falloff using range
float attenuate(float dist, float range)
{
    float x = clamp(1.0 - (dist * dist) / (range * range), 0.0, 1.0);
    return x * x;
}

// phong (diffuse + specular) for a single light source, with per-material ks/ns
vec3 phong(vec3 N, vec3 V, vec3 L, vec3 lightColor, vec3 baseColor, vec3 specColor, float shininess)
{
    float ndl = max(dot(N, L), 0.0);
    vec3 diffuse = baseColor * lightColor * ndl;

    vec3 R = reflect(-L, N);
    float spec = pow(max(dot(R, V), 0.0), max(shininess, 1.0));
    vec3 specular = lightColor * specColor * spec * step(0.0, ndl);

    return diffuse + specular;
}

// pcf 3x3 shadow lookup, returns visibility 0..1 (1 = fully lit)
float sampleShadow(sampler2D shadowMap, vec4 lightSpacePos, float ndl)
{
    // perspective divide and remap from [-1,1] to [0,1]
    vec3 ndc = lightSpacePos.xyz / lightSpacePos.w;
    ndc = ndc * 0.5 + 0.5;

    // outside the shadow map view frustum -> fully lit (border color = 1)
    if (ndc.z > 1.0) return 1.0;

    // slope-scaled bias to fight acne
    float bias = max(0.005 * (1.0 - ndl), 0.0008);

    vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));
    float shadow = 0.0;
    for (int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
            float depth = texture(shadowMap, ndc.xy + vec2(x, y) * texelSize).r;
            shadow += (ndc.z - bias > depth) ? 0.0 : 1.0;
        }
    }
    return shadow / 9.0;
}

void main()
{
    vec3 N = normalize(vNor);
    vec3 V = normalize(uCameraPos - vWorldPos);

    // kd, diffuse base (color * map_Kd)
    vec3 base = uColor;
    if (uHasAlbedo != 0)
    {
        base *= texture(uAlbedo, vUv).rgb;
    }

    // ks, specular color (material * map_Ks)
    vec3 specColor = uMatSpecular;
    if (uHasSpecMap != 0)
    {
        specColor *= texture(uSpecularMap, vUv).rgb;
    }

    float shininess = uMatShininess;

    // ambient + emissive
    vec3 color = base * uAmbientColor + uMatEmissive;

    // directional
    if (uHasDirLight != 0)
    {
        vec3 L = normalize(uDirLightDir);
        float ndl = max(dot(N, L), 0.0);

        float vis = 1.0;
        if (uDirCastsShadow != 0)
        {
            vec4 lsp = uDirLightVP * vec4(vWorldPos, 1.0);
            vis = sampleShadow(uDirShadowMap, lsp, ndl);
        }

        color += phong(N, V, L, uDirLightColor, base, specColor, shininess) * vis;
    }

    // points (no shadows)
    for (int i = 0; i < uPointCount; i++)
    {
        vec3 toLight = uPointLights[i].position - vWorldPos;
        float dist = length(toLight);
        vec3 L = toLight / max(dist, 1e-4);
        float att = attenuate(dist, uPointLights[i].range);
        color += phong(N, V, L, uPointLights[i].color, base, specColor, shininess) * att;
    }

    // spots
    for (int i = 0; i < uSpotCount; i++)
    {
        vec3 toLight = uSpotLights[i].position - vWorldPos;
        float dist = length(toLight);
        vec3 L = toLight / max(dist, 1e-4);

        float cosTheta = dot(-L, normalize(uSpotLights[i].direction));
        float coneFactor = smoothstep(uSpotLights[i].outerCos, uSpotLights[i].innerCos, cosTheta);
        float att = attenuate(dist, uSpotLights[i].range) * coneFactor;

        float vis = 1.0;
        if (uSpotCastsShadow[i] != 0 && att > 0.0)
        {
            float ndl = max(dot(N, L), 0.0);
            vec4 lsp = uSpotLightVP[i] * vec4(vWorldPos, 1.0);
            vis = sampleShadow(uSpotShadowMap[i], lsp, ndl);
        }

        color += phong(N, V, L, uSpotLights[i].color, base, specColor, shininess) * att * vis;
    }

    FragColor = vec4(color, 1.0);
}
