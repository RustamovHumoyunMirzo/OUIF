$input v_color0, v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(s_source, 0);
uniform vec4 u_blur;

void main()
{
    vec2 halfTexel = u_blur.xy * 0.5;
    vec4 color = texture2D(s_source, v_texcoord0 + vec2(-halfTexel.x, -halfTexel.y));
    color += texture2D(s_source, v_texcoord0 + vec2( halfTexel.x, -halfTexel.y));
    color += texture2D(s_source, v_texcoord0 + vec2(-halfTexel.x,  halfTexel.y));
    color += texture2D(s_source, v_texcoord0 + vec2( halfTexel.x,  halfTexel.y));
    gl_FragColor = color * 0.25;
}
