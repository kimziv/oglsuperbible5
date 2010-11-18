#version 330
precision highp float;

layout (location = 0) in vec4 position;
layout (location = 1) in vec4 instance_color;
layout (location = 2) in vec4 instance_position;

uniform float angle = 0.0;

out Fragment
{
    flat vec4 color;
} fragment;

void main(void)
{
    float ca = cos(angle);
    float sa = sin(angle);

    mat4 m = mat4(vec4(ca, -sa, 0.0, 0.0),
                  vec4(sa, ca, 0.0f, 0.0),
                  vec4(0.0, 0.0, 0.0, 0.0),
                  vec4(0.0, 0.0, 0.0, 1.0));

    gl_Position = (position + instance_position) * vec4(0.1, 0.1, 1.0, 1.0) * m;
    fragment.color = instance_color;
}
