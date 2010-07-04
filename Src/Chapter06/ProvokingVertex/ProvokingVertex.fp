// ProvokingVertex flat shader demo
// Fragment Shader
// Richard S. Wright Jr.
// OpenGL SuperBible
#version 130

out vec4 vFragColor;
flat in vec4 vVaryingColor;

void main(void)
   { 
   vFragColor = vVaryingColor;
   }