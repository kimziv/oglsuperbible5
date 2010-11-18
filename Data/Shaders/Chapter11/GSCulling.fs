// Simple culling Geometry Shader Example - Vertex Shader
// Vertex Shader
// Graham Sellers.
// OpenGL SuperBible 5th Ed.
#version 150

precision highp float;

in vec4 color;

out vec4 output_color;

void main(void)
{
    output_color = color;
}
