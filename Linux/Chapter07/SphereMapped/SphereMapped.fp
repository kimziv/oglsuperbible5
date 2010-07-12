// ADS Point lighting Shader
// Fragment Shader
// Richard S. Wright Jr.
// OpenGL SuperBible
#version 330

out vec4 vFragColor;

uniform sampler2D  sphereMap;

smooth in vec2 vVaryingTexCoord;

void main(void)
    { 
    vFragColor = texture(sphereMap, vVaryingTexCoord);
    }
    