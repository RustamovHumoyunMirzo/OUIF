$input v_color0, v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(s_source, 0);
uniform vec4 u_blur;

void main()
{
    vec2 d = u_blur.xy * max(u_blur.z, 0.5);
    vec4 color = (
        texture2D(s_source, v_texcoord0 + vec2(-d.x, -d.y)) +
        texture2D(s_source, v_texcoord0 + vec2( d.x, -d.y)) +
        texture2D(s_source, v_texcoord0 + vec2(-d.x,  d.y)) +
        texture2D(s_source, v_texcoord0 + vec2( d.x,  d.y))) * 0.0833333;

    color += (
        texture2D(s_source, v_texcoord0 + vec2(-2.0 * d.x, 0.0)) +
        texture2D(s_source, v_texcoord0 + vec2( 2.0 * d.x, 0.0)) +
        texture2D(s_source, v_texcoord0 + vec2(0.0, -2.0 * d.y)) +
        texture2D(s_source, v_texcoord0 + vec2(0.0,  2.0 * d.y))) * 0.1666666;

    gl_FragColor = mix(color, v_color0, v_color0.a);
}
