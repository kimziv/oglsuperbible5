// Normal Visualizer
// Geometry Shader
// Graham Sellers
// OpenGL SuperBible
#version 150

precision highp float;

layout (triangles) in;
layout (triangle_strip, max_vertices = 6) out;

in Vertex
{
    vec3 normal;
    vec4 color;
} vertex[];

out vec4 color;

uniform vec3 vLightPosition;
uniform mat4 mvpMatrix;
uniform mat4 mvMatrix;
uniform mat3 normalMatrix;

void main(void)
{
    int n;
    vec3 a = normalize(gl_in[0].gl_Position.xyz);
    vec3 b = normalize(gl_in[1].gl_Position.xyz);
    vec3 c = normalize(gl_in[2].gl_Position.xyz);

    vec3 d = normalize(b + c);

    gl_Position = mvpMatrix * vec4(b, 1.0);
    color = vec4(1.0, 0.0, 0.0, 1.0);
    EmitVertex();
    gl_Position = mvpMatrix * vec4(d, 1.0);
    color = vec4(0.0, 1.0, 0.0, 1.0);
    EmitVertex();
    gl_Position = mvpMatrix * vec4(a, 1.0);
    color = vec4(0.0, 0.0, 1.0, 1.0);
    EmitVertex();
    gl_Position = mvpMatrix * vec4(c, 1.0);
    color = vec4(1.0, 0.0, 1.0, 1.0);
    EmitVertex();

    EndPrimitive();
}