$input a_position, a_normal, a_texcoord0, a_color0
$output v_texcoord0, v_color0, v_normal, v_worldPos

#include <bgfx_shader.sh>

uniform vec4 u_diffuseColor;
uniform vec4 u_specularColor;
uniform vec4 u_emissiveColor;
uniform vec4 u_ambientColor;
uniform vec4 u_shininess;
uniform vec4 u_lightingEnable;

// Light structures - up to 8 lights
uniform vec4 u_lightPos[8];
uniform vec4 u_lightDir[8];
uniform vec4 u_lightColor[8];
uniform vec4 u_lightAtten[8];  // x=range, y=falloff, z=spot angle, w=type

void main()
{
    vec3 worldPos = mul(u_model[0], vec4(a_position, 1.0)).xyz;
    v_worldPos = worldPos;
    
    vec3 worldNormal = normalize(mul(u_model[0], vec4(a_normal, 0.0)).xyz);
    v_normal = worldNormal;
    
    v_texcoord0 = a_texcoord0;
    v_color0 = a_color0;
    
    gl_Position = mul(u_viewProj, vec4(worldPos, 1.0));
}
