// Perspective correction demonstration
// Fragment Shader
// Graham Sellers
// OpenGL SuperBible
#version 150

precision highp float;

in vec2 texcoord;
noperspective in vec2 texcoordNoPerspective;

out vec4 output_color;

uniform sampler2D grid;
uniform int usePerspective;

void main(void)
{
    vec2 coord;

    if (usePerspective != 0)
        coord = texcoord;
    else
        coord = texcoordNoPerspective;

    output_color = vec4(vec3(texture(grid, coord).r), 1.0);
}
