// Normal visualizer
// Fragment Shader
// Graham Sellers
// OpenGL SuperBible
#version 150

precision highp float;

in Fragment
{
    vec4 color;
} fragment;

out vec4 output_color;

void main(void)
{
    output_color = fragment.color;
}
