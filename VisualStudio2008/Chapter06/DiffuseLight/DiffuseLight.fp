// Simple Diffuse lighting Shader
// Fragment Shader
// Richard S. Wright Jr.
// OpenGL SuperBible
#version 330

layout (location=0) out vec4 vFragColor;
smooth in vec4 vVaryingColor;

void main(void)
   { 
   vFragColor = vVaryingColor;
   }