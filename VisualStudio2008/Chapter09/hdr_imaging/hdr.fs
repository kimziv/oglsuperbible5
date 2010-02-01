// hdr.fs
// 
// 
#version 150 
varying vec2 vTexCoord;
uniform sampler2D textureUnit0;
void main(void) 
{ 
	// fetch from HDR texture
	vec4 hdrTexel = texture2D(textureUnit0, vTexCoord); 
	
	gl_FragColor = hdrTexel;
}
