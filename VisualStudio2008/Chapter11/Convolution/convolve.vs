#version 150
precision highp float;

in vec2 position;

out Fragment
{
    vec2 tex_coord;
} fragment;

void main(void)
{
    gl_Position = vec4(position, 0.5, 1.0);

    // This produces a texture coordinate that ranges from (0.0, 0.0) to (1.0, 1.0)
    fragment.tex_coord = position * 0.5 + vec2(0.5, 0.5);
}
