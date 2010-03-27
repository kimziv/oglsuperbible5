// ADS Point lighting Shader
// Fragment Shader
// Richard S. Wright Jr.
// OpenGL SuperBible
#version 130

uniform sampler1D colorTable;

out vec4 gl_FragColor;
in vec4 vVaryingColor;
in float textureCoordinate;


void main(void)
   { 
   gl_FragColor = texture1D(colorTable, textureCoordinate);
   }