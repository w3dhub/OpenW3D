$input v_texcoord0, v_color0, v_normal, v_worldPos
$output gl_FragColor

#include <bgfx_shader.sh>

uniform vec4 u_diffuseColor;
uniform vec4 u_specularColor;
uniform vec4 u_emissiveColor;
uniform vec4 u_ambientColor;
uniform vec4 u_shininess;
uniform vec4 u_lightingEnable;
uniform vec4 u_opacity;
uniform vec4 u_alphaTest;

// Light environment — up to 4 lights (directional/point/spot)
uniform vec4 u_ambientLight;
uniform vec4 u_lightDir[4];   // xyz=direction, w=type (1=dir, 2=point, 3=spot)
uniform vec4 u_lightDiffuse[4]; // xyz=color, w=enable
uniform vec4 u_lightPos[4];   // xyz=position, w=range
uniform vec4 u_camPos;

// Fog
uniform vec4 u_fogColor;
uniform vec4 u_fogParams; // x=start, y=end, z=density, w=enable

SAMPLER2D(s_diffuse, 0);

vec3 CalculateLighting(vec3 worldPos, vec3 normal, vec3 diffuseColor, vec3 specularColor, float shininess)
{
    vec3 result = vec3_splat(0.0);

    if (u_lightingEnable.x < 0.5)
    {
        return diffuseColor;
    }

    // Ambient term
    vec3 ambient = u_ambientLight.xyz * diffuseColor;
    result += ambient;

    vec3 viewDir = normalize(u_camPos.xyz - worldPos);

    for (int i = 0; i < 4; i++)
    {
        // Light enabled if diffuse alpha > 0
        if (u_lightDiffuse[i].w < 0.5) continue;

        vec3 lightColor = u_lightDiffuse[i].xyz;
        vec3 lightDir;
        float attenuation = 1.0;
        int lightType = int(u_lightDir[i].w + 0.5);

        if (lightType == 2) // Point
        {
            vec3 toLight = u_lightPos[i].xyz - worldPos;
            float dist = length(toLight);
            lightDir = normalize(toLight);
            float range = u_lightPos[i].w;
            if (dist > range) continue;
            attenuation = 1.0 - (dist / range);
            attenuation = max(attenuation, 0.0);
            attenuation = pow(attenuation, 2.0);
        }
        else // Directional (type 1) or default
        {
            lightDir = -normalize(u_lightDir[i].xyz);
        }

        // Diffuse
        float NdotL = max(dot(normal, lightDir), 0.0);
        vec3 diff = diffuseColor * lightColor * NdotL;

        // Specular (Blinn-Phong)
        vec3 halfDir = normalize(lightDir + viewDir);
        float NdotH = max(dot(normal, halfDir), 0.0);
        float specFactor = pow(NdotH, shininess);
        vec3 spec = specularColor * lightColor * specFactor;

        result += (diff + spec) * attenuation;
    }

    return result;
}

void main()
{
    vec4 texColor = texture2D(s_diffuse, v_texcoord0);
    vec4 diffuse = texColor * u_diffuseColor;

    // Alpha test
    if (u_alphaTest.x > 0.5 && diffuse.a < u_alphaTest.y)
    {
        discard;
    }

    vec3 litColor = CalculateLighting(v_worldPos, normalize(v_normal), diffuse.xyz, u_specularColor.xyz, u_shininess.x);

    vec3 finalColor = litColor + u_emissiveColor.xyz;
    float alpha = diffuse.a * u_opacity.x;

    // Fog
    if (u_fogParams.w > 0.5)
    {
        float dist = length(u_camPos.xyz - v_worldPos);
        float fogFactor = (u_fogParams.y - dist) / (u_fogParams.y - u_fogParams.x);
        fogFactor = clamp(fogFactor, 0.0, 1.0);
        finalColor = mix(u_fogColor.rgb, finalColor, fogFactor);
    }

    gl_FragColor = vec4(finalColor, alpha);
}
