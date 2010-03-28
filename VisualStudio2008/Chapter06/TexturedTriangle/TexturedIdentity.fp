// The TexturedIdentity Shader
// Fragment Shader
// Richard S. Wright Jr.
// OpenGL SuperBible
#version 130


uniform sampler2D colorMap;

out vec4 gl_FragColor; 
in vec4 vVaryingTexCoords;


void main(void)
   { 
   gl_FragColor = texture2D(colorMap, vVaryingTexCoords.st);
   }