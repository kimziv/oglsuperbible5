// ADS Point lighting Shader
// Fragment Shader
// Richard S. Wright Jr.
// OpenGL SuperBible
//#version 330


uniform samplerCube cubeMap;
varying vec3 vVaryingTexCoord;

void main(void)
    { 
    gl_FragColor = textureCube(cubeMap, vVaryingTexCoord.stp);
    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    }
    