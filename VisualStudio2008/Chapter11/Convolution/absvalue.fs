#version 150
precision highp float;

// Input interface block from vertex shader
in Fragment
{
    vec2 tex_coord;
} fragment;

//
uniform sampler2D tex_input_image;

out vec4 output_color;

void main(void)
{
    output_color = abs(texture(tex_input_image, fragment.tex_coord));
}
