// Perspective correction demonstration
// Vertex Shader
// Graham Sellers
// OpenGL SuperBible
#version 150

precision highp float;

// Incoming per vertex position and texture coordinate
in vec4 vVertex;
in vec2 vTexCoord;

// Regular (perspective correct) texture coordinate
out vec2 texcoord;
// noperspective texture coordinate
noperspective out vec2 texcoordNoPerspective;

uniform mat4 mvpMatrix;

void main(void)
{
    // Dot product gives us diffuse intensity
    texcoord = vTexCoord;
    texcoordNoPerspective = vTexCoord;
    gl_Position = mvpMatrix * vVertex;
}
