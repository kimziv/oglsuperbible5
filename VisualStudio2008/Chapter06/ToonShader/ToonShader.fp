// Cell lighting Shader
// Fragment Shader
// Richard S. Wright Jr.
// OpenGL SuperBible
#version 330

uniform sampler1D colorTable;

layout (location=0) out vec4 vFragColor;

smooth in float textureCoordinate;


void main(void)
   { 
   vFragColor = texture(colorTable, textureCoordinate);
   }