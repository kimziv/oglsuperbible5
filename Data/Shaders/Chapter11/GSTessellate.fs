// Normal visualizer
// Fragment Shader
// Graham Sellers
// OpenGL SuperBible
#version 150

precision highp float;

in vec4 color;

out vec4 output_color;

void main(void)
{
    output_color = color;
}
