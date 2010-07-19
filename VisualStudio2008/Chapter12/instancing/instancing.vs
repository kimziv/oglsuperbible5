#version 150
precision highp float;

in vec4 position;
in vec4 instance_color;
in vec4 instance_position;

out Fragment
{
    vec4 color;
} fragment;

void main(void)
{
    gl_Position = (position + instance_position) * vec4(0.1, 0.1, 1.0, 1.0);
    fragment.color = instance_color;
}
