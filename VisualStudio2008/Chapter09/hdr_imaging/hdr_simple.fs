// hdr.fs
// Simple texture replace
// 
#version 150 
varying vec2 vTex;
uniform sampler2D textureUnit0;
out vec4 oColor;

void main(void) 
{ 
	// fetch from HDR texture
	vec4 hdrTexel = texture2D(textureUnit0, vTex); 
	
	oColor = hdrTexel;
}
