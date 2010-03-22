// The ShadedIdentity Shader
// Fragment Shader
// Richard S. Wright Jr.
// OpenGL SuperBible
#version 140

out vec4 gl_FragColor;
centroid in vec4 vVaryingColor;

void main(void)
   { 
   gl_FragColor = vVaryingColor;
   }