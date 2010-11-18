#version 330
precision highp float;

in Fragment
{
    flat vec4 color;
} fragment;

out vec4 color;

void main(void)
{
    color = fragment.color;
}
