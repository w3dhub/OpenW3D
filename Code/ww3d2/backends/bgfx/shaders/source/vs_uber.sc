$input a_position, a_normal, a_texcoord0, a_color0
$input a_texcoord1, a_texcoord2, a_texcoord3, a_color1
$output v_texcoord0, v_texcoord1, v_texcoord2, v_texcoord3
$output v_color0, v_color1, v_normal, v_worldPos

#include <bgfx_shader.sh>

// Custom uniforms (avoid names defined in bgfx_shader.sh)
uniform vec4 u_uberModel[4];
uniform vec4 u_uberViewProj[4];
uniform vec4 u_uberCamPos;
uniform vec4 u_uberLightingEnable;

void main()
{
    vec4 pos = vec4(a_position, 1.0);
    vec4 worldPos = vec4(
        dot(u_uberModel[0], pos),
        dot(u_uberModel[1], pos),
        dot(u_uberModel[2], pos),
        dot(u_uberModel[3], pos)
    );
    v_worldPos = worldPos.xyz;

    vec4 normal = vec4(a_normal, 0.0);
    v_normal = vec3(
        dot(u_uberModel[0], normal),
        dot(u_uberModel[1], normal),
        dot(u_uberModel[2], normal)
    );

    v_texcoord0 = a_texcoord0;
    v_texcoord1 = a_texcoord1;
    v_texcoord2 = a_texcoord2;
    v_texcoord3 = a_texcoord3;

    v_color0 = a_color0;
    v_color1 = a_color1;

    vec4 viewProjPos = vec4(
        dot(u_uberViewProj[0], worldPos),
        dot(u_uberViewProj[1], worldPos),
        dot(u_uberViewProj[2], worldPos),
        dot(u_uberViewProj[3], worldPos)
    );
    gl_Position = viewProjPos;
}
