// MoonShader
// Fragment Shader
// Richard S. Wright Jr.
// OpenGL SuperBible
#version 330
#extension GL_EXT_gpu_shader4 : enable

out vec4 vFragColor;

uniform sampler2DArray moonImage;

smooth in vec3 vMoonCoords;

void main(void)
   { 
   vFragColor = texture2DArray(moonImage, vMoonCoords);
   }