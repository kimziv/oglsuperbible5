// Flat Shader
// Fragment Shader
// Richard S. Wright Jr.
// OpenGL SuperBible
#version 330

// Make geometry solid
uniform vec4 vColorValue;

// Output fragment color
layout (location=0) out vec4 vFragColor;


void main(void)
   { 
   vFragColor = vColorValue;
   }