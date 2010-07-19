#version 150
precision highp float;

in Fragment
{
    vec4 color;
} fragment;

out vec4 color;

void main(void)
{
    color = fragment.color;
}
