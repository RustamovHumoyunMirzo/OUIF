$input v_color0, v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(s_font, 0);

void main()
{
    vec4 sample = texture2D(s_font, v_texcoord0);
    gl_FragColor = vec4(v_color0.rgb, v_color0.a * sample.a);
}
