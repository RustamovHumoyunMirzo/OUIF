$input v_color0, v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(s_source, 0);
uniform vec4 u_blur;

void main()
{
    vec2 texel = u_blur.xy;
    float radius = max(u_blur.z, 0.0);
    vec2 stepv = texel * radius;

    vec4 color = texture2D(s_source, v_texcoord0) * 0.2270270270;
    color += texture2D(s_source, v_texcoord0 + stepv * vec2( 1.3846153846, 0.0)) * 0.3162162162;
    color += texture2D(s_source, v_texcoord0 - stepv * vec2( 1.3846153846, 0.0)) * 0.3162162162;
    color += texture2D(s_source, v_texcoord0 + stepv * vec2( 3.2307692308, 0.0)) * 0.0702702703;
    color += texture2D(s_source, v_texcoord0 - stepv * vec2( 3.2307692308, 0.0)) * 0.0702702703;

    color += texture2D(s_source, v_texcoord0 + stepv * vec2(0.0,  1.3846153846)) * 0.1581081081;
    color += texture2D(s_source, v_texcoord0 - stepv * vec2(0.0,  1.3846153846)) * 0.1581081081;
    color += texture2D(s_source, v_texcoord0 + stepv * vec2(0.0,  3.2307692308)) * 0.0351351351;
    color += texture2D(s_source, v_texcoord0 - stepv * vec2(0.0,  3.2307692308)) * 0.0351351351;

    color *= 0.70;
    gl_FragColor = mix(color, v_color0, v_color0.a);
}
