#version 150 
// hdr.fs
// Simple texture replace
// 

in vec2 vTexCoord;

uniform sampler2D textureUnit0;

out vec4 oColor;

void main(void) 
{ 
	// fetch from HDR texture
	vec4 hdrTexel = texture(textureUnit0, vTexCoord); 
	
	oColor = hdrTexel;
}
