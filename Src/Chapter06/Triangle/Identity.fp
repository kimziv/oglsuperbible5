// The Identity Shader
// Fragment Shader
// Richard S. Wright Jr.
// OpenGL SuperBible
#version 150

out vec4 gl_FragColor;

uniform vec4 vColor;

void main(void)
   { 
   gl_FragColor = vColor;
   }