// Cell lighting Shader
// Vertex Shader
// Richard S. Wright Jr.
// OpenGL SuperBible
#version 150

precision highp float;

// Incoming per vertex... position and normal
in vec4 vVertex;
in vec3 vNormal;

out Vertex
{
    vec3 normal;
} vertex;

void main(void)
{
    gl_Position = vVertex;
    vertex.normal = vNormal;
}
