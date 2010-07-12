#version 150 
// blur.vs
// outputs: position and texture coord 0
// 

uniform mat4 mvpMatrix;
in vec3 vVertex;
in vec2 texCoord0;
out vec2 vTexCoord;

void main(void) 
{ 
	vTexCoord = texCoord0;
	gl_Position = mvpMatrix * vec4(vVertex, 1.0); 
}