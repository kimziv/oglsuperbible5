// Flat Shader
// Fragment Shader
// Richard S. Wright Jr.
// OpenGL SuperBible
#version 330

// Make geometry solid
uniform vec4 vColorValue;

// Output fragment color
out vec4 vFragColor;


void main(void)
   { 
   vFragColor = vColorValue;
   }