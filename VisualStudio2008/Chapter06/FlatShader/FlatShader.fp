// Flat Shader
// Fragment Shader
// Richard S. Wright Jr.
// OpenGL SuperBible
#version 130

// Make geometry solid
uniform vec4 vColorValue;

// Output fragment color
out vec4 gl_FragColor;


void main(void)
   { 
   gl_FragColor = vColorValue;
   }