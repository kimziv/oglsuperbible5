#version 150
precision highp float;

in vec3 position;
in vec3 normal;

out Fragment
{
    vec4 color;
} fragment;

uniform mat4 mvp;

void main(void)
{
    gl_Position = mvp * vec4(position + vec3(0.0, 0.0, -60.0), 1.0);
    fragment.color = vec4(abs(normal), 1.0);
}
