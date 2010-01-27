// blur.vs
// outputs: position and texture coord 0
// 

#version 150 

uniform mat4 mvpMatrix;
attribute vec3 vVertex;
attribute vec2 texCoord0;
varying vec2 vTexCoord;

void main(void) 
{ 
	vTexCoord = texCoord0;
	gl_Position = mvpMatrix * vec4(vVertex, 1.0); 
}