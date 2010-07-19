#version 150
precision highp float;

// Input interface block from vertex shader
in Fragment
{
    vec2 tex_coord;
} fragment;

//
uniform sampler2D tex_input_image;

uniform samplerBuffer tbo_coefficient;

uniform vec2 tc_scale;

uniform int kernel_size;

out vec4 output_color;

void main(void)
{
    int filter_size = kernel_size; // textureSize(tbo_coefficient);
    vec4 color = vec4(0.0);
    vec2 tc_offset;
    float coefficient;

    for (int i = 0; i < filter_size; i++) {
        coefficient = texelFetch(tbo_coefficient, i).r;
        tc_offset = float(i - filter_size / 2) * tc_scale;
        color += coefficient * texture(tex_input_image, fragment.tex_coord + tc_offset);
    }

    // output_color = texture(tex_input_image, fragment.tex_coord) * texelFetch(tbo_coefficient, 3).r;
    output_color = color;
}
