// Perspective correction demonstration
// Vertex Shader
// Graham Sellers
// OpenGL SuperBible
#version 150

precision highp float;

// Incoming per vertex position and texture coordinate
in vec4 vVertex;

// Zoom factor
uniform float zoom;

// Offset vector
uniform vec2 offset;

out vec2 initial_z;

void main(void)
{
    initial_z = (vVertex.xy * zoom) + offset;
    gl_Position = vVertex;
}
