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

// Light structures - up to 8 lights
uniform vec4 u_lightPos[8];
uniform vec4 u_lightDir[8];
uniform vec4 u_lightColor[8];
uniform vec4 u_lightAtten[8];  // x=range, y=falloff, z=spot angle, w=type

SAMPLER2D(s_diffuse, 0);

vec3 CalculateLighting(vec3 worldPos, vec3 normal, vec3 diffuseColor, vec3 specularColor, float shininess)
{
    vec3 result = vec3_splat(0.0);
    
    if (u_lightingEnable.x < 0.5)
    {
        return diffuseColor;
    }
    
    vec3 ambient = u_ambientColor.xyz * diffuseColor;
    result += ambient;
    
    vec3 viewDir = normalize(u_camPos.xyz - worldPos);
    
    for (int i = 0; i < 8; i++)
    {
        int lightType = int(u_lightAtten[i].w);
        if (lightType == 0) continue; // Light disabled
        
        vec3 lightColor = u_lightColor[i].xyz;
        vec3 lightDir;
        float attenuation = 1.0;
        float spotFactor = 1.0;
        
        if (lightType == 1) // Directional
        {
            lightDir = -normalize(u_lightDir[i].xyz);
        }
        else if (lightType == 2) // Point
        {
            vec3 toLight = u_lightPos[i].xyz - worldPos;
            float dist = length(toLight);
            lightDir = normalize(toLight);
            
            float range = u_lightAtten[i].x;
            if (dist > range) continue;
            
            attenuation = 1.0 - (dist / range);
            attenuation = max(attenuation, 0.0);
            attenuation = pow(attenuation, u_lightAtten[i].y);
        }
        else if (lightType == 3) // Spot
        {
            vec3 toLight = u_lightPos[i].xyz - worldPos;
            float dist = length(toLight);
            lightDir = normalize(toLight);
            
            float range = u_lightAtten[i].x;
            if (dist > range) continue;
            
            attenuation = 1.0 - (dist / range);
            attenuation = max(attenuation, 0.0);
            
            float spotAngle = u_lightAtten[i].z;
            float spotCos = dot(-lightDir, normalize(u_lightDir[i].xyz));
            spotFactor = smoothstep(cos(spotAngle), 1.0, spotCos);
        }
        
        float NdotL = max(dot(normal, lightDir), 0.0);
        vec3 diffuse = lightColor * diffuseColor * NdotL;
        
        vec3 halfDir = normalize(lightDir + viewDir);
        float NdotH = max(dot(normal, halfDir), 0.0);
        vec3 specular = lightColor * specularColor * pow(NdotH, shininess * 128.0);
        
        result += (diffuse + specular) * attenuation * spotFactor;
    }
    
    return result;
}

void main()
{
    vec4 texColor = texture2D(s_diffuse, v_texcoord0);
    vec4 diffuse = texColor * u_diffuseColor;
    
    vec3 litColor = CalculateLighting(v_worldPos, normalize(v_normal), diffuse.xyz, u_specularColor.xyz, u_shininess.x);
    
    vec3 finalColor = litColor + u_emissiveColor.xyz;
    float alpha = diffuse.a * u_opacity.x;
    
    gl_FragColor = vec4(finalColor, alpha);
}
